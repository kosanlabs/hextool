#ifndef PATTERN_H
#define PATTERN_H

#include <stdbool.h>
#include <stddef.h>

bool pattern_init_hex(const char* str);
bool pattern_init_ascii(const char* str);
bool pattern_is_active(void);
bool pattern_match_at(const unsigned char* buffer, size_t buf_len, size_t pos);
void pattern_compute_highlights(const unsigned char* prev, size_t prev_len,
                                const unsigned char* cur, size_t cur_len,
                                const unsigned char* next, size_t next_len,
                                bool* highlight);

#endif  // !PATTERN_H
