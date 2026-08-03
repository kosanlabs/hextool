#ifndef PATTERN_H
#define PATTERN_H

#include <stdbool.h>
#include <stddef.h>

extern unsigned char g_pattern[256];
extern size_t g_pat_len;
extern bool g_has_pattern;

bool parse_hex_pattern(const char* str);
bool parse_ascii_pattern(const char* str);
bool is_match_at(const unsigned char* buffer, size_t buf_len, size_t pos);
bool is_match_at(const unsigned char* buffer, size_t buf_len, size_t pos);

#endif  // !PATTERN_H
