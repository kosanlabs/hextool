#ifndef DISASM_H
#define DISASM_H

#include <stddef.h>
#include <stdint.h>

size_t disasm_x86_64(const uint8_t* buf, size_t len, size_t offset, char* out, size_t out_len);

#endif  // !DISASM_H
