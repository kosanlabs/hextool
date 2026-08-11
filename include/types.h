#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stddef.h>

#include "elf.h"

typedef struct {
  double mean;
  double m2;
  double min;
  double max;
  size_t count;
  size_t anomaly_high;
  size_t anomaly_low;
} EntropyStats;

typedef struct {
  size_t total_bytes;
  size_t total_lines;
  size_t null_count;
  size_t print_count;
  size_t high_count;
  bool is_elf;
  unsigned short elf_machine;
  EntropyStats entropy;
  size_t freq[256];
  ElfSegment segments[MAX_SEGMENTS];
  int segment_count;
} DumpStatistik;

#endif  // !TYPES_H
