#include "include/elf.h"

bool detect_elf(FILE* file) {
  unsigned char magic[4];
  size_t n = fread(magic, 1, 4, file);
  rewind(file);
  return (n == 4 && magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F');
}

unsigned short read_elf_machine(FILE* file) {
  unsigned char buf[2];
  if (fseek(file, 18, SEEK_SET) != 0) {
    return 0;
  }
  if (fread(buf, 1, 2, file) != 2) {
    return 0;
  }
  rewind(file);
  return (unsigned short)(buf[0] | (buf[1] << 8));
}

const char* machine_to_arch(unsigned short machine) {
  switch (machine) {
    case 0x00:
      return "No specific";
    case 0x02:
      return "SPARC";
    case 0x03:
      return "x86";
    case 0x08:
      return "MIPS";
    case 0x14:
      return "PowerPC";
    case 0x15:
      return "PowerPC64";
    case 0x16:
      return "S390";
    case 0x28:
      return "ARM";
    case 0x2A:
      return "SuperH";
    case 0x32:
      return "IA-64";
    case 0x3E:
      return "x86-64";
    case 0xB7:
      return "AArch64";
    case 0xF3:
      return "RISC-V";
    default:
      return "Unknown";
  }
}
