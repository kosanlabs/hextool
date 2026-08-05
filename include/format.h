#ifndef FORMAT_H
#define FORMAT_H

#include <stdbool.h>
#include <stddef.h>

#include "types.h"

void print_offset(size_t offset);

double calc_entropy(const unsigned char* buf, size_t n);
void entropy_update(EntropyStats* s, double e);
void print_entropy_bar(double entropy, EntropyStats* stats);
void print_le32(const unsigned char* buf, size_t n);
void print_hex(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset,
               const bool* highlight);
void print_ascii(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset,
                 const bool* highlight);
void print_inline_strings(const unsigned char* buffer, size_t bytes_read);
char printable_char(unsigned char c);

#endif  // !FORMAT_H
