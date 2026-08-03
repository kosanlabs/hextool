#include "include/format.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>

#include "include/color.h"
#include "include/config.h"
#include "include/pattern.h"

char printable_char(unsigned char c) {
  return isprint(c) ? (char)c : '.';
}

void print_offset(size_t offset) {
  printf(CLR_OFFSET "%08zx " CLR_RESET " ", offset);
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

void print_entropy_bar(const unsigned char* buf, size_t n) {
  double e = calc_entropy(buf, n);
  int filled = (int)((e / 8.0) * ENTROPY_BAR_WIDTH + 0.5);
  if (filled > ENTROPY_BAR_WIDTH) {
    filled = ENTROPY_BAR_WIDTH;
  }

  printf(CLR_ENTROPY "[");
  for (int i = 0; i < ENTROPY_BAR_WIDTH; i++) {
    putchar(i < filled ? '|' : ' ');
  }
  printf("]" CLR_RESET " ");
}

void print_le32(const unsigned char* buf, size_t n) {
  if (n >= 4) {
    unsigned int val = (unsigned int)buf[0] | ((unsigned int)buf[1] << 8) |
                       ((unsigned int)buf[2] << 16) | ((unsigned int)buf[3] << 24);
    printf(CLR_LE "|%08x|" CLR_RESET " ", val);
  } else {
    printf("           ");
  }
}

void print_hex(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset) {
  for (size_t i = 0; i < BYTES_PER_LINE; i++) {
    if (i == 8) {
      putchar(' ');
    }
    if (i < bytes_read) {
      bool elf_pos = is_elf && (offset + i) < ELF_MAGIC_SIZE;
      bool match = pattern_is_active() && pattern_match_at(buffer, bytes_read, i);
      printf("%s%02X" CLR_RESET " ", char_color(buffer[i], elf_pos, match),
             (unsigned int)buffer[i]);
    } else {
      printf("   ");
    }
  }
}

void print_ascii(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset) {
  printf(CLR_ASCII "|" CLR_RESET);
  for (size_t i = 0; i < bytes_read; i++) {
    bool elf_pos = is_elf && (offset + i) < ELF_MAGIC_SIZE;
    bool match = pattern_is_active() && pattern_match_at(buffer, bytes_read, i);
    printf("%s%c" CLR_RESET, char_color(buffer[i], elf_pos, match), printable_char(buffer[i]));
  }
  for (size_t i = bytes_read; i < BYTES_PER_LINE; i++) {
    putchar(' ');
  }
  printf(CLR_ASCII "|" CLR_RESET);
}

void print_inline_strings(const unsigned char* buffer, size_t bytes_read) {
  size_t best_start = 0, best_len = 0;
  size_t cur_start = 0, cur_len = 0;

  for (size_t i = 0; i < bytes_read; i++) {
    if (isprint(buffer[i])) {
      if (cur_len == 0) cur_start = i;
      cur_len++;
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

  if (best_len >= MIN_STRING_LEN) {
    printf(" " CLR_DIM "str:" CLR_RESET " " CLR_STR "\"");
    for (size_t i = 0; i < best_len && (best_start + i) < bytes_read; i++) {
      putchar(buffer[best_start + i]);
    }
    printf("\"" CLR_RESET);
  }
}
