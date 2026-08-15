#ifndef DUMP_H
#define DUMP_H

#include <stdio.h>

#include "types.h"

bool dump_file(FILE* file, DumpStatistik* stats, long start_offset, size_t max_length,
               bool big_endian, bool disasm);

#endif  // !DUMP_H
