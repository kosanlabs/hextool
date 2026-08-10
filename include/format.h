#ifndef FORMAT_H
#define FORMAT_H

#include <stdbool.h>
#include <stddef.h>

#include "config.h"
#include "types.h"

void print_offset(size_t offset);

double calc_entropy(const unsigned char* buf, size_t n);
void entropy_update(EntropyStats* s, double e);
void print_entropy_bar(double entropy, EntropyStats* stats, const char** out_anomaly_mark,
                       const char** out_bar_color);
void print_le32(const unsigned char* buf, size_t n);
void print_be32(const unsigned char* buf, size_t n);
void print_hex(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset,
               const bool* highlight);
void print_ascii(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset,
                 const bool* highlight);
void print_inline_strings(const unsigned char* buffer, size_t bytes_read);
char printable_char(unsigned char c);

typedef struct {
  unsigned char buf[ENTROPY_WINDOW_SIZE];
  size_t len;
  size_t pos;
} EntropyWindow;

void entropy_window_init(EntropyWindow* w);
void entropy_window_push(EntropyWindow* w, const unsigned char* data, size_t n);
double entropy_window_value(const EntropyWindow* w);

#endif  // !FORMAT_H
