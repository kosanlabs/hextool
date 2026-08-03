#include "include/color.h"

#include <ctype.h>

#include "include/config.h"

const char* byte_color(unsigned char chr, bool elf_magic_pos, bool is_match) {
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

const char* ascii_color(unsigned char chr, bool elf_magic_pos, bool is_match) {
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
