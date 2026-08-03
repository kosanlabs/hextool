#ifndef PATTERN_H
#define PATTERN_H

#include <stdbool.h>
#include <stddef.h>

extern unsigned char g_pattern[256];
extern size_t g_pat_len;
extern bool g_has_pattern;

bool pattern_init_hex(const char* str);
bool pattern_init_ascii(const char* str);
bool pattern_is_active(void);
bool pattern_match_at(const unsigned char* buffer, size_t buf_len, size_t pos);

#endif  // !PATTERN_H
