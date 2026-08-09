#include "include/diff.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "include/color.h"
#include "include/config.h"
#include "include/format.h"

static const char* diff_color(void) {
  return color_is_enabled() ? CLR_DIFF : "";
}

static void print_diff_line(const unsigned char* buf1, size_t len1, const unsigned char* buf2,
                            size_t len2, size_t offset) {
  bool diff[BYTES_PER_LINE] = {false};
  size_t max_len = len1 > len2 ? len1 : len2;

  for (size_t i = 0; i < max_len && i < BYTES_PER_LINE; i++) {
    unsigned char b1 = i < len1 ? buf1[i] : 0;
    unsigned char b2 = i < len2 ? buf2[i] : 0;
    diff[i] = (i >= len1 || i >= len2 || b1 != b2);
  }

  printf("%s%08zx%s  ", AC(CLR_OFFSET), offset, AC(CLR_RESET));
  for (size_t i = 0; i < BYTES_PER_LINE; i++) {
    if (i == 8) putchar(' ');
    if (i < len1) {
      const char* c = diff[i] ? diff_color() : char_color(buf1[i], false, false);
      printf("%s%02X%s ", c, (unsigned)buf1[i], AC(CLR_RESET));
    } else {
      printf("   ");
    }
  }
  printf(" |");
  for (size_t i = 0; i < len1; i++) {
    const char* c = diff[i] ? diff_color() : char_color(buf1[i], false, false);
    printf("%s%c%s", c, printable_char(buf1[i]), AC(CLR_RESET));
  }
  for (size_t i = len1; i < BYTES_PER_LINE; i++) {
    putchar(' ');
  }
  printf("|  A\n");

  printf("%s%08zx%s  ", AC(CLR_OFFSET), offset, AC(CLR_RESET));
  for (size_t i = 0; i < BYTES_PER_LINE; i++) {
    if (i == 8) putchar(' ');
    if (i < len2) {
      const char* c = diff[i] ? diff_color() : char_color(buf2[i], false, false);
      printf("%s%02X%s ", c, (unsigned)buf2[i], AC(CLR_RESET));
    } else {
      printf("   ");
    }
  }
  printf(" |");
  for (size_t i = 0; i < len2; i++) {
    const char* c = diff[i] ? diff_color() : char_color(buf2[i], false, false);
    printf("%s%c%s", c, printable_char(buf2[i]), AC(CLR_RESET));
  }
  for (size_t i = len2; i < BYTES_PER_LINE; i++) {
    putchar(' ');
  }
  printf("|  B\n");
}

int diff_files(const char* path_a, const char* path_b) {
  FILE* file_a = fopen(path_a, "rb");
  FILE* file_b = fopen(path_b, "rb");

  if (!file_a) {
    perror(path_a);
    return -1;
  }

  if (!file_b) {
    perror(path_b);
    return -1;
  }

  unsigned char buf1[BYTES_PER_LINE];
  unsigned char buf2[BYTES_PER_LINE];
  size_t offset = 0;
  size_t total_diffs = 0;
  size_t first_diff = (size_t)-1;

  while (1) {
    size_t n1 = fread(buf1, 1, BYTES_PER_LINE, file_a);
    size_t n2 = fread(buf2, 1, BYTES_PER_LINE, file_b);

    if (n1 == 0 && n2 == 0) {
      break;
    }

    if (ferror(file_a) || ferror(file_b)) {
      perror("fread");
      fclose(file_a);
      fclose(file_b);
      return -1;
    }

    bool line_diff = false;
    size_t max_n = n1 > n2 ? n1 : n2;

    for (size_t i = 0; i < max_n; i++) {
      unsigned char b1 = i < n1 ? buf1[i] : 0;
      unsigned char b2 = i < n2 ? buf2[i] : 0;
      if (i >= n1 || i >= n2 || b1 != b2) {
        line_diff = true;
        total_diffs++;
        if (first_diff == (size_t)-1) first_diff = offset + i;
      }
    }

    if (line_diff) {
      print_diff_line(buf1, n1, buf2, n2, offset);
      putchar('\n');
    }

    offset += BYTES_PER_LINE;
  }

  fclose(file_a);
  fclose(file_b);

  printf("\nDiff Detail:\n");
  printf("  Total differing bytes: %zu\n", total_diffs);

  if (first_diff != (size_t)-1) {
    printf("  First difference at offset: 0x%08zx\n", first_diff);
    return 1;
  }

  printf("  Files are identical.\n");
  return 0;
}
