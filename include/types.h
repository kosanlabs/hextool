#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
  size_t total_bytes;
  size_t total_lines;
  size_t null_count;
  size_t print_count;
  size_t high_count;
  bool is_elf;
  unsigned short elf_machine;
} DumpStatistik;

#endif // !TYPES_H
