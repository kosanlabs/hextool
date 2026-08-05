#include "include/pattern.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "include/config.h"

static unsigned char s_pattern[MAX_PATTERN_LEN];
static size_t s_pat_len = 0;
static bool s_has_pattern = false;

bool pattern_init_hex(const char* str) {
  s_pat_len = 0;
  size_t len = strlen(str);
  size_t i = 0;

  while (i < len && s_pat_len < MAX_PATTERN_LEN) {
    while (i < len && isspace((unsigned char)str[i])) i++;
    if (i >= len) break;

    unsigned int byte;
    if (sscanf(str + i, "%2x", &byte) == 1) {
      s_pattern[s_pat_len++] = (unsigned char)byte;
      i += 2;
      while (i < len && isspace((unsigned char)str[i])) i++;
    } else {
      s_has_pattern = false;
      return false;
    }
  }
  s_has_pattern = s_pat_len > 0;
  return s_has_pattern;
}

bool pattern_init_ascii(const char* str) {
  s_pat_len = strlen(str);
  if (s_pat_len == 0 || s_pat_len > MAX_PATTERN_LEN) {
    s_has_pattern = false;
    return false;
  }
  memcpy(s_pattern, str, s_pat_len);
  s_has_pattern = true;
  return true;
}

bool pattern_is_active(void) {
  return s_has_pattern;
}

bool pattern_match_at(const unsigned char* buffer, size_t buf_len, size_t pos) {
  if (!s_has_pattern || pos + s_pat_len > buf_len) return false;
  return memcmp(buffer + pos, s_pattern, s_pat_len) == 0;
}

void pattern_compute_highlights(const unsigned char* prev, size_t prev_len,
                                const unsigned char* cur, size_t cur_len,
                                const unsigned char* next, size_t next_len,
                                bool* highlight) {
  if (!s_has_pattern || cur_len == 0) {
    return;
  }

  unsigned char win[3 * BYTES_PER_LINE];
  size_t wlen = 0;
  if (prev) {
    memcpy(win, prev, prev_len);
    wlen += prev_len;
  }
  size_t cur_off = wlen;
  memcpy(win + wlen, cur, cur_len);
  wlen += cur_len;
  if (next) {
    memcpy(win + wlen, next, next_len);
    wlen += next_len;
  }

  for (size_t pos = 0; pos < cur_len; pos++) {
    if (cur_off + pos + s_pat_len <= wlen &&
        memcmp(win + cur_off + pos, s_pattern, s_pat_len) == 0) {
      highlight[pos] = true;
    }
  }
}
