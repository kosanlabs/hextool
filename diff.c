#include "include/diff.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "include/color.h"
#include "include/config.h"
#include "include/format.h"

static const char* clr_rm(void) {
  return color_is_enabled() ? CLR_DIFF : "";
}

static const char* clr_add(void) {
  return color_is_enabled() ? CLR_DIFF_ADD : "";
}

static void print_rm_line(const unsigned char* buf, size_t len, size_t offset, const bool* diff) {
  printf("%s-%s %s%08zx%s  ", clr_rm(), AC(CLR_RESET), AC(CLR_OFFSET), offset, AC(CLR_RESET));
  for (size_t i = 0; i < BYTES_PER_LINE; i++) {
    if (i == 8) putchar(' ');
    if (i < len) {
      const char* c = diff[i] ? clr_rm() : char_color(buf[i], false, false);
      printf("%s%02X%s ", c, (unsigned)buf[i], AC(CLR_RESET));
    } else {
      printf("   ");
    }
  }
  printf(" |");
  for (size_t i = 0; i < len; i++) {
    const char* c = diff[i] ? clr_rm() : char_color(buf[i], false, false);
    printf("%s%c%s", c, printable_char(buf[i]), AC(CLR_RESET));
  }
  for (size_t i = len; i < BYTES_PER_LINE; i++) putchar(' ');
  printf("|\n");
}

static void print_add_line(const unsigned char* buf, size_t len, size_t offset, const bool* diff) {
  printf("%s+%s %s%08zx%s  ", clr_add(), AC(CLR_RESET), AC(CLR_OFFSET), offset, AC(CLR_RESET));
  for (size_t i = 0; i < BYTES_PER_LINE; i++) {
    if (i == 8) putchar(' ');
    if (i < len) {
      const char* c = diff[i] ? clr_add() : char_color(buf[i], false, false);
      printf("%s%02X%s ", c, (unsigned)buf[i], AC(CLR_RESET));
    } else {
      printf("   ");
    }
  }
  printf(" |");
  for (size_t i = 0; i < len; i++) {
    const char* c = diff[i] ? clr_add() : char_color(buf[i], false, false);
    printf("%s%c%s", c, printable_char(buf[i]), AC(CLR_RESET));
  }
  for (size_t i = len; i < BYTES_PER_LINE; i++) putchar(' ');
  printf("|\n");
}

static void print_ctx_line(const unsigned char* buf, size_t len, size_t offset) {
  printf("  %s%08zx%s  ", AC(CLR_OFFSET), offset, AC(CLR_RESET));
  for (size_t i = 0; i < BYTES_PER_LINE; i++) {
    if (i == 8) putchar(' ');
    if (i < len) {
      printf("%s%02X%s ", char_color(buf[i], false, false), (unsigned)buf[i], AC(CLR_RESET));
    } else {
      printf("   ");
    }
  }
  printf(" |");
  for (size_t i = 0; i < len; i++) {
    printf("%s%c%s", char_color(buf[i], false, false), printable_char(buf[i]), AC(CLR_RESET));
  }
  for (size_t i = len; i < BYTES_PER_LINE; i++) putchar(' ');
  printf("|\n");
}

int diff_files(const char* path_a, const char* path_b) {
  FILE* fa = fopen(path_a, "rb");
  FILE* fb = fopen(path_b, "rb");
  if (!fa) {
    perror(path_a);
    return -1;
  }
  if (!fb) {
    perror(path_b);
    fclose(fa);
    return -1;
  }

  printf("--- %s\n", path_a);
  printf("+++ %s\n", path_b);

  unsigned char buf_a[BYTES_PER_LINE];
  unsigned char buf_b[BYTES_PER_LINE];
  unsigned char ctx[BYTES_PER_LINE];
  size_t ctx_len = 0, ctx_off = 0;
  bool have_ctx = false, in_block = false;

  size_t offset = 0;
  size_t total_diffs = 0;
  size_t first_diff = (size_t)-1;

  while (1) {
    size_t n1 = fread(buf_a, 1, BYTES_PER_LINE, fa);
    size_t n2 = fread(buf_b, 1, BYTES_PER_LINE, fb);
    if (n1 == 0 && n2 == 0) break;
    if (ferror(fa) || ferror(fb)) {
      perror("fread");
      fclose(fa);
      fclose(fb);
      return -1;
    }

    bool diff[BYTES_PER_LINE] = {false};
    bool line_diff = false;
    size_t max_n = n1 > n2 ? n1 : n2;
    for (size_t i = 0; i < max_n && i < BYTES_PER_LINE; i++) {
      unsigned char b1 = i < n1 ? buf_a[i] : 0;
      unsigned char b2 = i < n2 ? buf_b[i] : 0;
      if (i >= n1 || i >= n2 || b1 != b2) {
        diff[i] = true;
        line_diff = true;
        total_diffs++;
        if (first_diff == (size_t)-1) first_diff = offset + i;
      }
    }

    if (line_diff) {
      if (have_ctx && !in_block) {
        print_ctx_line(ctx, ctx_len, ctx_off);
      }
      print_rm_line(buf_a, n1, offset, diff);
      print_add_line(buf_b, n2, offset, diff);
      putchar('\n');
      in_block = true;
    } else {
      memcpy(ctx, buf_a, n1);
      ctx_len = n1;
      ctx_off = offset;
      have_ctx = true;
      in_block = false;
    }

    offset += BYTES_PER_LINE;
  }

  fclose(fa);
  fclose(fb);

  printf("Diff summary:\n");
  printf("  Total differing bytes: %zu\n", total_diffs);
  if (first_diff != (size_t)-1) {
    printf("  First difference at offset: 0x%08zx\n", first_diff);
    return 1;
  }
  printf("  Files are identical.\n");
  return 0;
}
