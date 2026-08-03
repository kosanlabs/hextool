#include "include/color.h"

#include <ctype.h>

#include "include/config.h"

const char* char_color(unsigned char chr, bool elf_magic_pos, bool is_match) {
  if (!color_is_enabled()) {
    return "";
  }
  if (is_match) {
    return CLR_MATCH;
  }
  if (elf_magic_pos) {
    return CLR_ELF;
  }
  if (chr == 0x00) {
    return CLR_NULL;
  }
  if (chr >= 0x80) {
    return CLR_HIGH;
  }
  if (isprint(chr)) {
    return CLR_PRINT;
  }
  return CLR_LOW;
}

static bool s_color_enabled = true;

bool color_is_enabled(void) {
  return s_color_enabled;
}

void color_disable(void) {
  s_color_enabled = false;
}
