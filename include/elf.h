#ifndef ELF_H
#define ELF_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_SEGMENTS 16

typedef struct {
  uint64_t offset;
  uint64_t filesz;
  uint32_t type;
  uint32_t flags;
} ElfSegment;

bool detect_elf(FILE* file);
unsigned short read_elf_machine(FILE* file);
const char* machine_to_arch(unsigned short machine);
const char* elf_field_name(size_t offset);

int elf_read_segments(FILE* file, ElfSegment* segs, int max_segs);
const char* elf_segment_type_name(uint32_t type);
const char* elf_segment_at(const ElfSegment* segs, int n, size_t offset);

#endif  // !ELF_H
