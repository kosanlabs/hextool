#include "include/format.h"

#include <ctype.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>

#include "include/color.h"
#include "include/config.h"
#include "include/pattern.h"

char printable_char(unsigned char c) {
  return isprint(c) ? (char)c : '.';
}

void print_offset(size_t offset) {
  printf("%s%08zx %s", AC(CLR_OFFSET), offset, AC(CLR_RESET));
}

double calc_entropy(const unsigned char* buf, size_t n) {
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

void entropy_window_init(EntropyWindow* w) {
  w->len = 0;
  w->pos = 0;
}

void entropy_window_push(EntropyWindow* w, const unsigned char* data, size_t n) {
  for (size_t i = 0; i < n; i++) {
    w->buf[w->pos] = data[i];
    w->pos = (w->pos + 1) % ENTROPY_WINDOW_SIZE;
    if (w->len < ENTROPY_WINDOW_SIZE) {
      w->len++;
    }
  }
}

double entropy_window_value(const EntropyWindow* w) {
  unsigned char linear[ENTROPY_WINDOW_SIZE];

  if (w->len < ENTROPY_WINDOW_SIZE) {
    memcpy(linear, w->buf, w->len);
  } else {
    size_t split = w->pos;
    memcpy(linear, w->buf + split, ENTROPY_WINDOW_SIZE - split);
    memcpy(linear + ENTROPY_WINDOW_SIZE - split, w->buf, split);
  }

  return calc_entropy(linear, w->len);
}

void entropy_update(EntropyStats* s, double e) {
  s->count++;
  double delta = e - s->mean;
  s->mean += delta / (double)s->count;
  s->m2 += delta * (e - s->mean);
  if (s->count == 1 || e < s->min) {
    s->min = e;
  }
  if (e > s->max) {
    s->max = e;
  }

  if (s->count > ENTROPY_WARMUP) {
    double variance = s->m2 / (double)s->count;
    double stddev = sqrt(variance);
    if (stddev >= 0.001) {
      double z = (e - s->mean) / stddev;
      if (z > 2.0) {
        s->anomaly_high++;
      }
      if (z < -2.0) {
        s->anomaly_low++;
      }
    }
  }
}

void print_entropy_bar(double entropy, EntropyStats* stats, const char** out_anomaly_mark,
                       const char** out_bar_color) {
  int filled;
  *out_anomaly_mark = "";
  *out_bar_color = AC(CLR_ENTROPY);

  if (stats->count < ENTROPY_WARMUP) {
    filled = (int)((entropy / 8.0) * ENTROPY_BAR_WIDTH + 0.5);
  } else {
    double variance = stats->m2 / (double)stats->count;
    double stddev = sqrt(variance);

    if (stddev < 0.001) {
      filled = ENTROPY_BAR_WIDTH / 2;
    } else {
      double z = (entropy - stats->mean) / stddev;
      filled = (int)(((z + 3.0) / 6.0) * ENTROPY_BAR_WIDTH + 0.5);

      if (z > 2.0) {
        *out_anomaly_mark = "!";
        *out_bar_color = AC(CLR_ENTROPY_HIGH);
      } else if (z < -2.0) {
        *out_anomaly_mark = ".";
        *out_bar_color = AC(CLR_ENTROPY_LOW);
      }
    }
  }

  if (filled < 0) {
    filled = 0;
  }
  if (filled > ENTROPY_BAR_WIDTH) {
    filled = ENTROPY_BAR_WIDTH;
  }

  const char* BL = "\xe2\x96\x91";
  const char* BM = "\xe2\x96\x92";
  const char* BH = "\xe2\x96\x93";

  printf("%s[%s", *out_bar_color, AC(CLR_RESET));
  for (int i = 0; i < ENTROPY_BAR_WIDTH; i++) {
    if (i < filled) {
      if (i < ENTROPY_BAR_WIDTH / 3) {
        fputs(BL, stdout);
      } else if (i < 2 * ENTROPY_BAR_WIDTH / 3) {
        fputs(BM, stdout);
      } else {
        fputs(BH, stdout);
      }
    } else {
      putchar(' ');
    }
  }
  printf("%s]%s%s ", *out_bar_color, AC(CLR_RESET),
         (*out_anomaly_mark)[0] ? *out_anomaly_mark : " ");
}

void print_le32(const unsigned char* buf, size_t n) {
  if (n >= 4) {
    unsigned int val = (unsigned int)buf[0] | ((unsigned int)buf[1] << 8) |
                       ((unsigned int)buf[2] << 16) | ((unsigned int)buf[3] << 24);
    printf("%s|%08x|%s ", AC(CLR_LE), val, AC(CLR_RESET));
  } else {
    printf("           ");
  }
}

void print_be32(const unsigned char* buf, size_t n) {
  if (n >= 4) {
    unsigned int val = ((unsigned int)buf[0] << 24) | ((unsigned int)buf[1] << 16) |
                       ((unsigned int)buf[2] << 8) | (unsigned int)buf[3];
    printf("%s|%08x|%s ", AC(CLR_LE), val, AC(CLR_RESET));
  } else {
    printf(" ");
  }
}

void print_hex(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset,
               const bool* highlight) {
  for (size_t i = 0; i < BYTES_PER_LINE; i++) {
    if (i == 8) {
      putchar(' ');
    }
    if (i < bytes_read) {
      bool elf_pos = is_elf && (offset + i) < ELF_MAGIC_SIZE;
      bool match = pattern_is_active() && highlight[i];
      printf("%s%02X%s ", char_color(buffer[i], elf_pos, match), (unsigned int)buffer[i],
             AC(CLR_RESET));
    } else {
      printf("   ");
    }
  }
}

void print_ascii(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset,
                 const bool* highlight) {
  printf("%s|%s", AC(CLR_ASCII), AC(CLR_RESET));
  for (size_t i = 0; i < bytes_read; i++) {
    bool elf_pos = is_elf && (offset + i) < ELF_MAGIC_SIZE;
    bool match = pattern_is_active() && highlight[i];
    printf("%s%c%s", char_color(buffer[i], elf_pos, match), printable_char(buffer[i]),
           AC(CLR_RESET));
  }
  for (size_t i = bytes_read; i < BYTES_PER_LINE; i++) {
    putchar(' ');
  }
  printf("%s|%s", AC(CLR_ASCII), AC(CLR_RESET));
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
    printf(" %sstr:%s %s\"", AC(CLR_DIM), AC(CLR_RESET), AC(CLR_STR));
    for (size_t i = 0; i < best_len && (best_start + i) < bytes_read; i++) {
      putchar(buffer[best_start + i]);
    }
    printf("\"%s", AC(CLR_RESET));
  }
}
