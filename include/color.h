#ifndef COLOR_H
#define COLOR_H

#include <stdbool.h>

const char* char_color(unsigned char chr, bool elf_magic_pos, bool is_match);
void color_disable(void);
bool color_is_enabled(void);

#define AC(code) (color_is_enabled() ? (code) : "")

#endif  // !COLOR_H
