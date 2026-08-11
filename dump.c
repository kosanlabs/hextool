#define _POSIX_C_SOURCE 200809L

#include "include/dump.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#ifdef __linux__
#include <fcntl.h>
#endif /* ifdef __linux__ */

#include "include/color.h"
#include "include/config.h"
#include "include/elf.h"
#include "include/format.h"
#include "include/pattern.h"

typedef struct {
  double entropy;
  const char* elf_field;
} LineAnalysis;

static LineAnalysis analyze_line(const unsigned char* buf, size_t len, size_t offset, bool is_elf,
                                 EntropyWindow* ewin) {
  LineAnalysis a;

  entropy_window_push(ewin, buf, len);
  a.entropy = entropy_window_value(ewin);

  if (is_elf && offset < ELF_HEADER_SIZE) {
    a.elf_field = elf_field_name(offset);
  } else {
    a.elf_field = NULL;
  }

  return a;
}

static void print_line(const unsigned char* buf, size_t len, size_t offset, bool is_elf,
                       const bool* highlight, EntropyStats* estate, bool big_endian,
                       const LineAnalysis* analysis) {
  print_offset(offset);

  entropy_update(estate, analysis->entropy);

  const char* anomaly_mark;
  const char* bar_color;
  print_entropy_bar(analysis->entropy, estate, &anomaly_mark, &bar_color);

  if (big_endian) {
    print_be32(buf, len);
  } else {
    print_le32(buf, len);
  }

  print_hex(buf, len, is_elf, offset, highlight);
  putchar(' ');
  print_ascii(buf, len, is_elf, offset, highlight);
  print_inline_strings(buf, len);

  if (analysis->elf_field) {
    printf(" %s; %s%s", AC(CLR_COMMENT), analysis->elf_field, AC(CLR_RESET));
  }

  putchar('\n');

  if (is_elf && offset < ELF_HEADER_SIZE) {
    const char* field = elf_field_name(offset);
    if (field) {
      printf(" %s; %s%s", AC(CLR_COMMENT), field, AC(CLR_RESET));
    }
  }
  putchar('\n');
}

bool dump_file(FILE* file, DumpStatistik* stats, long start_offset, size_t max_length,
               bool big_endian) {
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

  if (start_offset == 0) {
    stats->is_elf = detect_elf(file);

    if (ferror(file)) {
      perror("fread");
      return false;
    }

    if (stats->is_elf) {
      stats->elf_machine = read_elf_machine(file);

      if (ferror(file)) {
        perror("fread");
        return false;
      }
      rewind(file);
    }
  } else {
    stats->is_elf = false;
  }

#ifdef __linux__
  posix_fadvise(fileno(file), start_offset, max_length ? (off_t)max_length : 0,
                POSIX_FADV_SEQUENTIAL);
#endif /* ifdef __linux__ */

  if (offset > 0) {
    if (fseek(file, offset, SEEK_SET) != 0) {
      perror("fseek");
      return false;
    }
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

      LineAnalysis analysis =
          analyze_line(bufs[print_idx], lens[print_idx], offsets[print_idx], stats->is_elf, &ewin);

      print_line(bufs[print_idx], lens[print_idx], offsets[print_idx], stats->is_elf, highlight,
                 &estate, big_endian, &analysis);

      if (ferror(stdout)) {
        return false;
      }
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

    LineAnalysis analysis =
        analyze_line(bufs[print_idx], lens[print_idx], offsets[print_idx], stats->is_elf, &ewin);

    print_line(bufs[print_idx], lens[print_idx], offsets[print_idx], stats->is_elf, highlight,
               &estate, big_endian, &analysis);

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
