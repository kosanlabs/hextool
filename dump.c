#include <ctype.h>
#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "include/dump.h"

#ifdef __linux__
#include <fcntl.h>
#endif

#include "include/binfmt.h"
#include "include/color.h"
#include "include/config.h"
#include "include/disasm.h"
#include "include/elf.h"
#include "include/format.h"
#include "include/pattern.h"

typedef struct {
  double entropy;
  const char* field_name;
  const char* segment;
} LineAnalysis;

static LineAnalysis analyze_line(const unsigned char* buf, size_t len, size_t offset,
                                 const BinFmtInfo* fmt, const ElfSegment* segs, int seg_count,
                                 EntropyWindow* ewin) {
  LineAnalysis a = {0};

  entropy_window_push(ewin, buf, len);
  a.entropy = entropy_window_value(ewin);

  a.field_name = binfmt_field_name(fmt->type, offset, fmt);

  if (fmt->type == FMT_ELF && seg_count > 0) {
    for (int i = 0; i < seg_count; i++) {
      if (offset == segs[i].offset) {
        a.segment = elf_segment_type_name(segs[i].type);
        break;
      }
    }
  }

  return a;
}

static void print_line(const unsigned char* buf, size_t len, size_t offset, bool is_elf,
                       const bool* highlight, EntropyStats* estate, bool big_endian,
                       const LineAnalysis* analysis) {
  (void)is_elf;

  print_offset(offset);

  entropy_update(estate, analysis->entropy);

  const char* anomaly_mark;
  const char* bar_color;
  print_entropy_bar(analysis->entropy, estate, &anomaly_mark, &bar_color);

  printf("%.2f", analysis->entropy);

  if (big_endian) {
    print_be32(buf, len);
  } else {
    print_le32(buf, len);
  }

  print_hex(buf, len, analysis->field_name != NULL, offset, highlight);
  putchar(' ');
  print_ascii(buf, len, analysis->field_name != NULL, offset, highlight);
  print_inline_strings(buf, len);

  if (analysis->field_name) {
    printf(" %s; %s%s", AC(CLR_COMMENT), analysis->field_name, AC(CLR_RESET));
  }
  if (analysis->segment) {
    printf(" %s; SEGMENT: %s%s", AC(CLR_COMMENT), analysis->segment, AC(CLR_RESET));
  }
  putchar('\n');
}

static void print_disasm_line(const unsigned char* buf, size_t len, size_t offset,
                              EntropyStats* estate, const LineAnalysis* analysis) {
  char mnemonic[64];
  size_t consumed = disasm_x86_64(buf, len, offset, mnemonic, sizeof(mnemonic));

  print_offset(offset);

  if (estate) {
    entropy_update(estate, analysis->entropy);
    const char* anomaly_mark;
    const char* bar_color;
    print_entropy_bar(analysis->entropy, estate, &anomaly_mark, &bar_color);
    printf("%.2f", analysis->entropy);
  } else {
    printf("                ");
  }

  size_t print_len = consumed > 0 ? consumed : 1;
  if (print_len > len) {
    print_len = len;
  }
  for (size_t i = 0; i < 7; i++) {
    if (i < print_len) {
      printf("%02x ", (unsigned)buf[i]);
    } else {
      printf("   ");
    }
  }

  printf(" %s%s%s", AC(CLR_DISASM), mnemonic, AC(CLR_RESET));

  if (analysis->field_name) {
    printf(" %s; %s%s", AC(CLR_COMMENT), analysis->field_name, AC(CLR_RESET));
  }
  if (analysis->segment) {
    printf(" %s; SEGMENT: %s%s", AC(CLR_COMMENT), analysis->segment, AC(CLR_RESET));
  }
  putchar('\n');
}

bool dump_file(FILE* file, DumpStatistik* stats, long start_offset, size_t max_length,
               bool big_endian, bool disasm) {
  unsigned char b0[BYTES_PER_LINE];
  unsigned char b1[BYTES_PER_LINE];
  unsigned char b2[BYTES_PER_LINE];
  unsigned char* bufs[3] = {b0, b1, b2};
  size_t lens[3] = {0, 0, 0};
  size_t offsets[3] = {0, 0, 0};
  int count = 0;

  EntropyStats estate = {0};
  EntropyWindow ewin;
  entropy_window_init(&ewin);

  size_t offset = (size_t)start_offset;
  size_t dumped = 0;

  if (start_offset < 0) {
    fprintf(stderr, "error: negative offset is not valid\n");
    return false;
  }

  BinFmtInfo fmt = {0};
  if (start_offset == 0) {
    binfmt_detect(file, &fmt);
    stats->format_type = fmt.type;
    stats->machine = fmt.machine;

    if (fmt.type == FMT_ELF) {
      stats->segment_count = elf_read_segments(file, stats->segments, MAX_SEGMENTS);
      rewind(file);
    }
  } else {
    stats->format_type = FMT_UNKNOWN;
  }

  if (disasm && !(fmt.type == FMT_ELF && (fmt.machine == 0x03 || fmt.machine == 0x3E)) &&
      !(fmt.type == FMT_PE && (fmt.machine == 0x014c || fmt.machine == 0x8664)) &&
      !(fmt.type == FMT_MACHO && (fmt.machine == 0x03 || fmt.machine == 0x3E))) {
    fprintf(stderr, "warning: disassembly mode works best with x86/x86-64 binaries\n");
  }

#ifdef __linux__
  posix_fadvise(fileno(file), start_offset, max_length ? (off_t)max_length : 0,
                POSIX_FADV_SEQUENTIAL);
#endif  // __linux__

  if (offset > 0) {
    if (fseek(file, offset, SEEK_SET) != 0) {
      perror("fseek");
      return false;
    }
  }

  if (!disasm) {
    print_dump_header();
  }

  if (disasm) {
    unsigned char inbuf[30];
    size_t fill = 0;

    while (1) {
      size_t want = 15 - fill;
      if (max_length > 0 && max_length - dumped < want) {
        want = max_length - dumped;
      }
      if (want == 0) break;

      size_t got = fread(inbuf + fill, 1, want, file);
      if (ferror(file)) {
        perror("fread");
        return false;
      }
      fill += got;
      if (got == 0) {
        break;
      }

      size_t pos = 0;
      while (pos + 15 <= fill) {
        LineAnalysis analysis = analyze_line(inbuf + pos, 15, offset + pos, &fmt, stats->segments,
                                             stats->segment_count, &ewin);

        print_disasm_line(inbuf + pos, 15, offset + pos, &estate, &analysis);

        char dummy[64];
        size_t step = disasm_x86_64(inbuf + pos, 15, offset + pos, dummy, sizeof(dummy));
        if (step == 0 || step > 15) {
          step = 1;
        }

        for (size_t i = 0; i < step; i++) {
          unsigned char c = inbuf[pos + i];
          stats->freq[c]++;
          if (c == 0x00) {
            stats->null_count++;
          } else if (c >= 0x80) {
            stats->high_count++;
          } else if (isprint(c)) {
            stats->print_count++;
          }
        }

        pos += step;
        stats->total_bytes += step;
        stats->total_lines++;
      }

      offset += pos;
      dumped += pos;
      memmove(inbuf, inbuf + pos, fill - pos);
      fill -= pos;
    }

    size_t pos = 0;
    while (pos < fill) {
      LineAnalysis analysis = analyze_line(inbuf + pos, fill - pos, offset + pos, &fmt,
                                           stats->segments, stats->segment_count, &ewin);
      print_disasm_line(inbuf + pos, fill - pos, offset + pos, &estate, &analysis);

      char dummy[64];
      size_t step = disasm_x86_64(inbuf + pos, fill - pos, offset + pos, dummy, sizeof(dummy));
      if (step == 0 || step > fill - pos) step = fill - pos;

      for (size_t i = 0; i < step; i++) {
        unsigned char c = inbuf[pos + i];
        stats->freq[c]++;
        if (c == 0x00) {
          stats->null_count++;
        } else if (c >= 0x80) {
          stats->high_count++;
        } else if (isprint(c)) {
          stats->print_count++;
        }
      }

      pos += step;
      stats->total_bytes += step;
      stats->total_lines++;
    }

    stats->entropy = estate;
    return !ferror(file);
  }

  while (1) {
    int idx = count % 3;
    size_t to_read = BYTES_PER_LINE;
    if (max_length > 0) {
      size_t remain = max_length - dumped;
      if (remain == 0) {
        break;
      }
      if (remain < to_read) {
        to_read = remain;
      }
    }

    lens[idx] = fread(bufs[idx], 1, to_read, file);
    if (lens[idx] == 0) {
      if (ferror(file)) {
        perror("fread");
        return false;
      }
      break;
    }
    offsets[idx] = offset;

    for (size_t i = 0; i < lens[idx]; i++) {
      unsigned char c = bufs[idx][i];
      stats->freq[c]++;
      if (c == 0x00) {
        stats->null_count++;
      } else if (c >= 0x80) {
        stats->high_count++;
      } else if (isprint(c)) {
        stats->print_count++;
      }
    }
    offset += lens[idx];
    dumped += lens[idx];
    stats->total_bytes += lens[idx];
    stats->total_lines++;

    if (count >= 1) {
      int print_idx = (count - 1) % 3;
      int prev_idx = (count >= 2) ? (count - 2) % 3 : -1;
      int next_idx = idx;

      bool highlight[BYTES_PER_LINE] = {false};
      pattern_compute_highlights(prev_idx >= 0 ? bufs[prev_idx] : NULL,
                                 prev_idx >= 0 ? lens[prev_idx] : 0, bufs[print_idx],
                                 lens[print_idx], bufs[next_idx], lens[next_idx], highlight);

      LineAnalysis analysis = analyze_line(bufs[print_idx], lens[print_idx], offsets[print_idx],
                                           &fmt, stats->segments, stats->segment_count, &ewin);

      print_line(bufs[print_idx], lens[print_idx], offsets[print_idx],
                 stats->format_type == FMT_ELF, highlight, &estate, big_endian, &analysis);

      if (ferror(stdout)) return false;
    }

    count++;
  }

  if (count >= 1) {
    int print_idx = (count - 1) % 3;
    int prev_idx = (count >= 2) ? (count - 2) % 3 : -1;

    bool highlight[BYTES_PER_LINE] = {false};
    pattern_compute_highlights(prev_idx >= 0 ? bufs[prev_idx] : NULL,
                               prev_idx >= 0 ? lens[prev_idx] : 0, bufs[print_idx], lens[print_idx],
                               NULL, 0, highlight);

    LineAnalysis analysis = analyze_line(bufs[print_idx], lens[print_idx], offsets[print_idx], &fmt,
                                         stats->segments, stats->segment_count, &ewin);

    print_line(bufs[print_idx], lens[print_idx], offsets[print_idx], stats->format_type == FMT_ELF,
               highlight, &estate, big_endian, &analysis);

    if (ferror(stdout)) {
      return false;
    }
  }

  stats->entropy = estate;
  if (ferror(file)) {
    perror("fread");
    return false;
  }
  return !ferror(file);
}
