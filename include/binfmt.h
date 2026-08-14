#ifndef BINFMT_H
#define BINFMT_H

#include <stdint.h>
#include <stdio.h>

enum { FMT_UNKNOWN = 0, FMT_ELF, FMT_PE, FMT_MACHO };

typedef struct {
  int type;
  unsigned short machine;
  uint32_t pe_offset;
} BinFmtInfo;

int binfmt_detect(FILE* file, BinFmtInfo* out);
const char* binfmt_name(int type);
const char* binfmt_machine_name(int type, unsigned short machine);
const char* binfmt_field_name(int type, size_t offset, const BinFmtInfo* info);

#endif  // !BINFMT_H
