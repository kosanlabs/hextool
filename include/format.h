#ifndef FORMAT_H
#define FORMAT_H

#include <stdbool.h>
#include <stddef.h>

void print_offset(size_t offset);
void print_entropy_bar(const unsigned char* buf, size_t n);
void print_le32(const unsigned char* buf, size_t n);
void print_hex(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset);
void print_ascii(const unsigned char* buffer, size_t bytes_read, bool is_elf, size_t offset);
void print_inline_strings(const unsigned char* buffer, size_t bytes_read);
const char* elf_field_name(size_t offset);
char printable_char(unsigned char c);

#endif  // !FORMAT_H
