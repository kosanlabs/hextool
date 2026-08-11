#include "test.h"

#include "../include/pattern.h"

TEST(pattern_hex_valid) {
  ASSERT(pattern_init_hex("7f454c46"));
  ASSERT(pattern_is_active());
  unsigned char buf[] = {0x7F, 'E', 'L', 'F', 0x00};
  ASSERT(pattern_match_at(buf, sizeof(buf), 0));
}

TEST(pattern_hex_with_spaces) {
  ASSERT(pattern_init_hex("7f 45 4c 46"));
  ASSERT(pattern_is_active());
}

TEST(pattern_hex_empty) {
  ASSERT(!pattern_init_hex(""));
  ASSERT(!pattern_is_active());
}

TEST(pattern_ascii_basic) {
  ASSERT(pattern_init_ascii("flag{"));
  ASSERT(pattern_is_active());
  unsigned char buf[] = "xxflag{123}";
  ASSERT(pattern_match_at(buf, sizeof(buf) - 1, 2));
  ASSERT(!pattern_match_at(buf, sizeof(buf) - 1, 0));
}

TEST(pattern_ascii_empty) {
  ASSERT(!pattern_init_ascii(""));
  ASSERT(!pattern_is_active());
}

TEST(pattern_cross_line_highlight) {
  ASSERT(pattern_init_ascii("FLAG{"));

  unsigned char prev[16] = {[14] = 'F', [15] = 'L'};
  unsigned char cur[16] = "AG{SECRET}";
  bool hl[16] = {false};
  pattern_compute_highlights(prev, 16, cur, 10, NULL, 0, hl);
  ASSERT(!hl[0]);

  unsigned char line[16] = {0};
  line[14] = 'F';
  line[15] = 'L';
  pattern_compute_highlights(NULL, 0, line, 16, cur, 10, hl);
  ASSERT(hl[14]);
  ASSERT(!hl[15]);
}

int main(void) {
  RUN(pattern_hex_valid);
  RUN(pattern_hex_with_spaces);
  RUN(pattern_hex_empty);
  RUN(pattern_ascii_basic);
  RUN(pattern_ascii_empty);
  RUN(pattern_cross_line_highlight);
  TEST_SUMMARY();
}
