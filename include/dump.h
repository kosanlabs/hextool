#ifndef DUMP_H
#define DUMP_H

#include <stdio.h>

#include "types.h"

bool dump_file(FILE* file, DumpStatistik* stats, long offset, size_t length);

#endif  // !DUMP_H
