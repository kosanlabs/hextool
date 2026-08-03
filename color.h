#ifndef COLOR_H
#define COLOR_H

#include <stdbool.h>

const char* byte_color(unsigned char chr, bool elf_magic_pos, bool is_match);
const char* ascii_color(unsigned char chr, bool elf_magic_pos, bool is_match);

#endif  // !COLOR_H
