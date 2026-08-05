#include "include/elf.h"

#include "include/config.h"

bool detect_elf(FILE* file) {
  unsigned char magic[ELF_MAGIC_SIZE];
  size_t n = fread(magic, 1, ELF_MAGIC_SIZE, file);
  rewind(file);
  return (n == ELF_MAGIC_SIZE && magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' &&
          magic[3] == 'F');
}

unsigned short read_elf_machine(FILE* file) {
  unsigned char buf[ELF_MACHINE_SIZE];
  if (fseek(file, ELF_MACHINE_OFFSET, SEEK_SET) != 0) {
    return 0;
  }
  if (fread(buf, 1, ELF_MACHINE_SIZE, file) != ELF_MACHINE_SIZE) {
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

const char* elf_field_name(size_t offset) {
  switch (offset) {
    case 0:
      return "e_ident[MAG0-3]";
    case 4:
      return "e_ident[CLASS]";
    case 5:
      return "e_ident[DATA]";
    case 6:
      return "e_ident[VERSION]";
    case 7:
      return "e_ident[OSABI]";
    case 8:
      return "e_ident[ABIVERSION]";
    case 16:
      return "e_type";
    case 18:
      return "e_machine";
    case 20:
      return "e_version";
    case 24:
      return "e_entry";
    case 32:
      return "e_phoff";
    case 40:
      return "e_shoff";
    case 48:
      return "e_flags";
    case 52:
      return "e_ehsize";
    case 54:
      return "e_phentsize";
    case 56:
      return "e_phnum";
    case 58:
      return "e_shentsize";
    case 60:
      return "e_shnum";
    case 62:
      return "e_shstrndx";
    default:
      if (offset >= 9 && offset < 16) {
        return "e_ident";
      }
      return NULL;
  }
}
