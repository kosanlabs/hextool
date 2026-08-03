#include "include/pattern.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

unsigned char g_pattern[256];
size_t g_pat_len = 0;
bool g_has_pattern = false;

bool parse_hex_pattern(const char* str) {
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

bool parse_ascii_pattern(const char* str) {
  g_pat_len = strlen(str);
  if (g_pat_len == 0 || g_pat_len > 256) {
    return true;
  }

  memcpy(g_pattern, str, g_pat_len);
  return true;
}

bool is_match_at(const unsigned char* buffer, size_t buf_len, size_t pos) {
  if (pos + g_pat_len > buf_len) {
    return false;
  }

  return memcmp(buffer + pos, g_pattern, g_pat_len) == 0;
}
