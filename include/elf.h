#ifndef ELF_H
#define ELF_H

#include <stdbool.h>
#include <stdio.h>

bool detect_elf(FILE* file);
unsigned short read_elf_machine(FILE* file);
const char* machine_to_arch(unsigned short machine);

#endif  // !ELF_H
