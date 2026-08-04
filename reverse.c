#include "include/reverse.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "include/config.h"

static const char* skip_ansi(const char* p) {
  if (*p == '\x1b' && *(p + 1) == '[') {
    p += 2;
  }

  while (*p && !((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z'))) {
    if (*p == '\0') {
      break;
    }

    p++;
  }

  if (*p) {
    p++;
  }

  return p;
}

static int hex_val(char chr) {
  if (chr >= '0' && chr <= '9') {
    return chr - '0';
  }
  if (chr >= 'A' && chr <= 'F') {
    return chr - 'A' + 10;
  }
  if (chr >= 'a' && chr <= 'f') {
    return chr - 'a' + 10;
  }
  return -1;
}

bool reverse_hexdump(FILE* in, FILE* out) {
  char line[2048];
  size_t line_no = 0;
  bool success = true;
  long current_pos = 0;

  while (fgets(line, sizeof(line), in)) {
    line_no++;
    const char* p = line;

    while (*p == ' ' || *p == '\t' || *p == '\x1b') {
      if (*p == '\x1b') {
        p = skip_ansi(p);
      } else {
        p++;
      }
    }

    if (strlen(p) < 8) {
      continue;
    }

    size_t offset = 0;
    int i = 0;

    for (i = 0; i < 8; i++) {
      int v = hex_val(p[i]);
      if (v < 0) {
        break;
      }
      offset = (offset << 4) | (size_t)v;
    }

    if (i < 8) {
      continue;
    }

    p += 8;

    const char* bracket_close = strchr(p, ']');
    if (bracket_close) {
      p = bracket_close + 1;
    }

    while (*p == ' ' || *p == '\x1b') {
      if (*p == '\x1b') {
        p = skip_ansi(p);
      } else {
        p++;
      }
    }

    if (*p == '|') {
      const char* end = p + 1;
      while (*end && *end != '|' && *end != '\n') {
        end++;
      }

      if (*end == '|' && (size_t)(end - p - 1) == 8) {
        bool all_hex = true;
        for (const char* q = p + 1; q < end; q++) {
          if (!isxdigit((unsigned char)*q)) {
            all_hex = false;
            break;
          }
        }

        if (all_hex) {
          p = end + 1;
        }
      }
    }

    unsigned char bytes[BYTES_PER_LINE];
    size_t byte_count = 0;

    while (*p && *p != '|' && *p != '\n' && byte_count < BYTES_PER_LINE) {
      if (*p == '\x1b') {
        p = skip_ansi(p);
        continue;
      }
      if (!isxdigit((unsigned char)*p)) {
        p++;
        continue;
      }
      int hi = hex_val(*p);
      if (hi < 0 || !*(p + 1)) {
        p++;
        continue;
      }
      int lo = hex_val(*(p + 1));
      if (lo < 0) {
        p++;
        continue;
      }
      bytes[byte_count++] = (unsigned char)((hi << 4) | lo);
      p += 2;
    }

    if (byte_count == 0) {
      continue;
    }

    if ((long)offset != current_pos) {
      if (fseek(out, (long)offset, SEEK_SET) != 0) {
        fprintf(stderr, "warning: cannot seek to offset 0x%08zx (line %zu)\n", offset, line_no);
        success = false;
        continue;
      }
    }

    if (fwrite(bytes, 1, byte_count, out) != byte_count) {
      fprintf(stderr, "warning: write error at offset 0x%08zx (line %zu)\n", offset, line_no);
      success = false;
      continue;
    }
    current_pos = (long)(offset + byte_count);
  }

  return success && !ferror(in);
}
