#ifndef STATS_H
#define STATS_H

#include "types.h"

void print_hasil(const char* filename, const DumpStatistik* stats);
static void print_entropy_summary(const EntropyStats* s);

#endif // !STATS_H
