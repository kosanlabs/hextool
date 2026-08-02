#include <ctype.h>
#include <limits.h>
#include <linux/limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ELF_HEADER_SIZE 64

enum { BYTES_PER_LINE = 16 };

#define CLR_RESET "\x1b[0m"
#define CLR_OFFSET "\x1b[35m"
#define CLR_NULL "\x1b[38;5;240m"
#define CLR_PRINT "\x1b[32m"
#define CLR_LOW "\x1b[31m"
#define CLR_HIGH "\x1b[33m"
#define CLR_ELF "\x1b[35;1m"
#define CLR_ASCII "\x1b[90m"
#define CLR_LE "\x1b[34m"
#define CLR_MATCH "\x1b[48;5;196m\x1b[37;1m"
#define CLR_STR "\x1b[38;5;124m"
#define CLR_ENTROPY "\x1b[38;5;39m"
#define CLR_BOLD "\x1b[1m"
#define CLR_DIM "\x1b[2m"

typedef struct {
  size_t total_bytes;
  size_t total_lines;
  size_t null_count;
  size_t print_count;
  size_t high_count;
  bool is_elf;
} DumpStatistik;

static unsigned char g_pattern[256];
static size_t g_pat_len = 0;
static bool g_has_pattern = false;

static FILE* open_file(const char* filename) {
  FILE* file = fopen(filename, "rb");

  if (file == NULL) {
    perror(filename);
  }

  return file;
}

static bool parse_pattern(const char* str) {
  g_pat_len = 0;
  size_t len = strlen(str);
  size_t i = 0;

  while (i < len && g_pat_len < 256) {
    while (i < len && isspace((unsigned char)str[i])) {
      i++;
    }

    if (i >= len) {
      break;
    }

    unsigned int byte;

    if (sscanf(str + i, "%2x", &byte) == 1) {
      g_pattern[g_pat_len++] = (unsigned char)byte;
      i += 2;

      while (i < len && isspace((unsigned char)str[i])) {
        i++;
      }
    } else {
      return false;
    }
  }

  return g_pat_len > 0;
}

static bool parse_ascii_pattern(const char* str) {
  g_pat_len = strlen(str);
  if (g_pat_len == 0 || g_pat_len > 256) {
    return true;
  }

  memcpy(g_pattern, str, g_pat_len);
  return true;
}

static bool is_match_at(const unsigned char* buffer, size_t buf_len, size_t pos) {
  if (pos + g_pat_len > buf_len) {
    return false;
  }

  return memcmp(buffer + pos, g_pattern, g_pat_len) == 0;
}

static const char* byte_color(unsigned char chr, bool elf_magic_pos, bool is_match) {
  if (is_match) {
    return CLR_MATCH;
  }

  if (elf_magic_pos) {
    return CLR_ELF;
  }

  if (chr == 0x00) {
    return CLR_NULL;
  }

  if (chr >= 0x80) {
    return CLR_HIGH;
  }

  if (isprint(chr)) {
    return CLR_PRINT;
  }

  return CLR_LOW;
}

static char printable_char(unsigned char c) {
  return isprint(c) ? (char)c : '.';
}

static const char* ascii_color(unsigned char chr, bool elf_magic_pos, bool is_match) {
  if (is_match) {
    return CLR_MATCH;
  }

  if (elf_magic_pos) {
    return CLR_ELF;
  }

  if (chr == 0x00) {
    return CLR_NULL;
  }

  if (chr >= 0x80) {
    return CLR_HIGH;
  }

  if (isprint(chr)) {
    return CLR_PRINT;
  }

  return CLR_LOW;
}

static void print_offset(size_t offset) {
  printf(CLR_OFFSET "%08zx  " CLR_RESET "    ", offset);
}

static double calc_entropy(const unsigned char* buf, size_t n) {
  if (n == 0) {
    return 0.0;
  }

  int freq[256] = {0};

  for (size_t i = 0; i < n; i++) {
    freq[buf[i]]++;
  }

  double entropy = 0.0;
  for (int i = 0; i < 256; i++) {
    if (freq[i] > 0) {
      double p = (double)freq[i] / (double)n;
      entropy -= p * log2(p);
    }
  }

  return entropy;
}

static void print_entropy_bar(const unsigned char* buf, size_t n) {
  double e = calc_entropy(buf, n);
  int filled = (int)((e / 8.0) * 8.0 + 0.5);

  if (filled > 8) {
    filled = 8;
  }

  printf(CLR_ENTROPY "[");
  for (int i = 0; i < 8; i++) {
    if (i < filled) {
      printf("|");
    } else {
      printf(" ");
    }
  }
  printf("]" CLR_RESET " ");
}

static void print_le32(const unsigned char* buf, size_t n) {
  if (n >= 4) {
    unsigned int val = (unsigned int)buf[0] | ((unsigned int)buf[1] << 8) |
                       ((unsigned int)buf[2] << 16) | ((unsigned int)buf[3] << 24);
    printf(CLR_LE "|%08x|" CLR_RESET "  ", val);
  } else {
    printf("          ");
  }
}

static void print_hex(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset) {
  for (size_t i = 0; i < BYTES_PER_LINE; i++) {
    if (i == 8) printf(" ");
    if (i < bytes_read) {
      bool elf_pos = is_elf && (offset + i) < 4;
      bool match = g_has_pattern && is_match_at(buffer, bytes_read, i);
      printf("%s%02X" CLR_RESET " ", byte_color(buffer[i], elf_pos, match));

    } else {
      printf("   ");
    }
  }
}

static void print_ascii(const unsigned char* buffer, size_t bytes_read, bool is_elf,
                        size_t offset) {
  printf(CLR_ASCII "|" CLR_RESET);

  for (size_t i = 0; i < bytes_read; i++) {
    bool elf_pos = is_elf && (offset + i) < 4;
    bool match = g_has_pattern && is_match_at(buffer, bytes_read, i);

    printf("%s%c" CLR_RESET, ascii_color(buffer[i], elf_pos, match), printable_char(buffer[i]));
  }

  for (size_t i = bytes_read; i < BYTES_PER_LINE; i++) {
    putchar(' ');
  }

  printf(CLR_ASCII "|" CLR_RESET);
}

static void print_inline_strings(const unsigned char* buffer, size_t bytes_read) {
  char str[BYTES_PER_LINE + 1];
  size_t best_start = 0, best_len = 0;
  size_t cur_start = 0, cur_len = 0;

  for (size_t i = 0; i < bytes_read; i++) {
    if (isprint(buffer[i])) {
      if (cur_len == 0) {
        cur_start = 1;
        cur_len++;
      }
    } else {
      if (cur_len > best_len) {
        best_len = cur_len;
        best_start = cur_start;
      }
      cur_len = 0;
    }
  }

  if (cur_len > best_len) {
    best_len = cur_len;
    best_start = cur_start;
  }

  if (best_len >= 4) {
    printf("  " CLR_DIM "str:" CLR_RESET " " CLR_STR "\"");
    for (size_t i = 0; i < best_len && (best_start + i) < bytes_read; i++) {
      putchar(buffer[best_start + i]);
    }
    printf("\"" CLR_RESET);
  }
}

static const char* elf_field_name(size_t offset) {
  if (offset == 0) return "e_ident[MAG0-3]";
  if (offset == 4) return "e_ident[CLASS]";
  if (offset == 5) return "e_ident[DATA]";
  if (offset == 6) return "e_ident[VERSION]";
  if (offset == 7) return "e_ident[OSABI]";
  if (offset == 8) return "e_ident[ABIVERSION]";
  if (offset >= 9 && offset < 16) return "e_ident";
  if (offset == 16) return "e_type";
  if (offset == 18) return "e_machine";
  if (offset == 20) return "e_version";
  if (offset == 24) return "e_entry";
  if (offset == 32) return "e_phoff";
  if (offset == 40) return "e_shoff";
  if (offset == 48) return "e_flags";
  if (offset == 52) return "e_ehsize";
  if (offset == 54) return "e_phentsize";
  if (offset == 56) return "e_phnum";
  if (offset == 58) return "e_shentsize";
  if (offset == 60) return "e_shnum";
  if (offset == 62) return "e_shstrndx";
  return NULL;
}

static void print_line(const unsigned char* buffer, size_t bytes_read, size_t offset, bool is_elf) {
  print_offset(offset);
  print_entropy_bar(buffer, bytes_read);
  print_le32(buffer, bytes_read);
  print_hex(buffer, bytes_read, is_elf, offset);
  printf("  ");
  print_ascii(buffer, bytes_read, is_elf, offset);
  print_inline_strings(buffer, bytes_read);

  if (is_elf && offset < ELF_HEADER_SIZE) {
    const char* field = elf_field_name(offset);
    if (field) printf("  \x1b[90m; %s\x1b[0m", field);
  }
  putchar('\n');
}

static void dump_file(FILE* file, DumpStatistik* stats) {
  unsigned char buffer[BYTES_PER_LINE];

  size_t offset = 0;
  size_t bytes_read;
  unsigned char magic[4];
  size_t magic_read = fread(magic, 1, 4, file);

  if (magic_read == 4 && magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' &&
      magic[3] == 'F') {
    stats->is_elf = true;
  }

  rewind(file);

  while ((bytes_read = fread(buffer, 1, BYTES_PER_LINE, file)) > 0) {
    print_line(buffer, bytes_read, offset, stats->is_elf);

    for (size_t i = 0; i < bytes_read; i++) {
      unsigned char chr = buffer[i];
      if (chr == 0x00) {
        stats->null_count++;
      } else if (chr >= 0x80) {
        stats->high_count++;
      } else if (isprint(chr)) {
        stats->print_count++;
      }
    }

    offset += bytes_read;
    stats->total_bytes += bytes_read;
    stats->total_lines++;
  }
}

static void print_hasil(const char* filename, const DumpStatistik* stats) {
  printf("\n\n");
  printf("File           : " CLR_BOLD "%s" CLR_RESET "\n", filename);

  printf("total bytesnya : %zu\n", stats->total_bytes);
  printf("total line     : %zu\n", stats->total_lines);

  if (stats->is_elf) {
    printf("Format         :" CLR_ELF " Elf Executable " CLR_RESET "\n");
  }

  if (g_has_pattern) {
    printf("Pattern        :" CLR_MATCH " SEARCH AKTIF " CLR_RESET "\n");
  }

  printf("\n");
  printf("Detail byte:\n");
  printf("    " CLR_NULL "#" CLR_RESET " Null bytes (0x00) : %zu (%.1f%%)\n", stats->null_count,
         stats->total_bytes ? (stats->null_count * 100.0 / stats->total_bytes) : 0);
  printf("    " CLR_PRINT "#" CLR_RESET " Printable ascii : %zu (%.1f%%)\n", stats->print_count,
         stats->total_bytes ? (stats->print_count * 100.0 / stats->total_bytes) : 0);
  printf("    " CLR_HIGH "#" CLR_RESET " High bytes (>= 0x80): %zu (%.1f%%)\n", stats->high_count,
         stats->total_bytes ? (stats->high_count * 100.0 / stats->total_bytes) : 0);
  printf("    " CLR_LOW "#" CLR_RESET " Control | Low bytes (>= 0x80): %zu (%.1f%%)\n",
         stats->total_bytes - stats->null_count - stats->print_count - stats->high_count,
         stats->total_bytes
             ? ((stats->total_bytes - stats->null_count - stats->print_count - stats->high_count) *
                100.0 / stats->total_bytes)
             : 0);
  printf("\n" CLR_DIM "Entropy bar: [| = high randomness, empty = low randomness]" CLR_RESET "\n");
}

static void print_usage(const char* prog) {
  fprintf(stderr,
          "penggunaan: %s [options] <file>\n"
          "\n"
          "Options:\n"
          "  -s <hex>    Search & highlight hex pattern (e.g., -s DEADBEEF)\n"
          "  -S <ascii>  Search & highlight ASCII pattern (e.g., -S \"flag\")\n"
          "\n"
          "Contoh:\n"
          "  %s /bin/ls\n"
          "  %s -s 7f454c46 /bin/ls        # cari magic ELF\n"
          "  %s -S \"flag{\" challenge.bin   # cari flag CTF\n",
          prog, prog, prog, prog);
}

int main(int argc, char* argv[]) {
  const char* filename = NULL;
  int arg_idx = 1;

  while (arg_idx < argc) {
    if (strcmp(argv[arg_idx], "-s") == 0 && arg_idx + 1 < argc) {
      if (!parse_pattern(argv[arg_idx + 1])) {
        fprintf(stderr, "error: pattern hex tidak valid\n");
        return EXIT_FAILURE;
      }
      g_has_pattern = true;
      arg_idx += 2;
    } else if (strcmp(argv[arg_idx], "-S") == 0 && arg_idx + 1 < argc) {
      if (!parse_ascii_pattern(argv[arg_idx + 1])) {
        fprintf(stderr, "error: pattern ASCII tidak valid\n");
        return EXIT_FAILURE;
      }
      g_has_pattern = true;
      arg_idx += 2;
    } else if (argv[arg_idx][0] == '-') {
      fprintf(stderr, "error: opsi tidak dikenal: %s\n", argv[arg_idx]);
      print_usage(argv[0]);
    } else {
      if (filename != NULL) {
        fprintf(stderr, "error: hanya boleh satu file\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
      }

      filename = argv[arg_idx];
      arg_idx++;
    }
  }

  if (filename == NULL) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  FILE* file = open_file(filename);
  if (file == NULL) return EXIT_FAILURE;

  DumpStatistik stats = {0};
  dump_file(file, &stats);
  fclose(file);
  print_hasil(filename, &stats);
  return 0;
}
