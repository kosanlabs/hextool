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

static uint16_t ru16(const unsigned char* p, bool be) {
  return be ? ((uint16_t)p[0] << 8 | p[1]) : ((uint16_t)p[1] << 8 | p[0]);
}

static uint32_t ru32(const unsigned char* p, bool be) {
  return be ? ((uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3])
            : ((uint32_t)p[3] << 24 | (uint32_t)p[2] << 16 | (uint32_t)p[1] << 8 | p[0]);
}

static uint64_t ru64(const unsigned char* p, bool be) {
  if (be) {
    return ((uint64_t)p[0] << 56 | (uint64_t)p[1] << 48 | (uint64_t)p[2] << 40 |
            (uint64_t)p[3] << 32 | (uint64_t)p[4] << 24 | (uint64_t)p[5] << 16 |
            (uint64_t)p[6] << 8 | (uint64_t)p[7]);
  }
  return ((uint64_t)p[7] << 56 | (uint64_t)p[6] << 48 | (uint64_t)p[5] << 40 |
          (uint64_t)p[4] << 32 | (uint64_t)p[3] << 24 | (uint64_t)p[2] << 16 | (uint64_t)p[1] << 8 |
          (uint64_t)p[0]);
}

int elf_read_segments(FILE* file, ElfSegment* segs, int max_segs) {
  unsigned char ident[16];
  if (fseek(file, 0, SEEK_SET) != 0) {
    return 0;
  }
  if (fread(ident, 1, 16, file) != 16) {
    return 0;
  }

  bool is_64 = (ident[4] == 2);
  bool be = (ident[5] == 2);

  unsigned char buf[8];
  uint64_t phoff;
  uint16_t phentsize, phnum;

  if (is_64) {
    if (fseek(file, 32, SEEK_SET) != 0) {
      return 0;
    }
    if (fread(buf, 1, 8, file) != 8) {
      return 0;
    }
    phoff = ru64(buf, be);

    if (fseek(file, 54, SEEK_SET) != 0) {
      return 0;
    }
    if (fread(buf, 1, 2, file) != 2) {
      return 0;
    }
    phentsize = ru16(buf, be);

    if (fread(buf, 1, 2, file) != 2) {
      return 0;
    }
    phnum = ru16(buf, be);
  } else {
    if (fseek(file, 28, SEEK_SET) != 0) {
      return 0;
    }
    if (fread(buf, 1, 4, file) != 4) {
      return 0;
    }
    phoff = ru32(buf, be);

    if (fseek(file, 42, SEEK_SET) != 0) {
      return 0;
    }
    if (fread(buf, 1, 2, file) != 2) {
      return 0;
    }
    phentsize = ru16(buf, be);

    if (fread(buf, 1, 2, file) != 2) {
      return 0;
    }
    phnum = ru16(buf, be);
  }

  if (phnum == 0) {
    rewind(file);
    return 0;
  }
  if (phnum > (uint16_t)max_segs) {
    phnum = (uint16_t)max_segs;
  }

  int count = 0;
  for (uint16_t i = 0; i < phnum && count < max_segs; i++) {
    uint64_t entry_off = phoff + (uint64_t)i * phentsize;
    if (fseek(file, (long)entry_off, SEEK_SET) != 0) {
      continue;
    }

    unsigned char ph[56];
    size_t ph_len = is_64 ? 56 : 32;
    if (fread(ph, 1, ph_len, file) != ph_len) {
      continue;
    }

    uint32_t type, flags;
    uint64_t p_offset, p_filesz;

    if (is_64) {
      type = ru32(ph, be);
      flags = ru32(ph + 4, be);
      p_offset = ru64(ph + 8, be);
      p_filesz = ru64(ph + 32, be);
    } else {
      type = ru32(ph, be);
      p_offset = ru32(ph + 4, be);
      p_filesz = ru32(ph + 16, be);
      flags = ru32(ph + 24, be);
    }

    segs[count].type = type;
    segs[count].offset = p_offset;
    segs[count].filesz = p_filesz;
    segs[count].flags = flags;
    count++;
  }

  rewind(file);
  return count;
}

const char* elf_segment_type_name(uint32_t type) {
  switch (type) {
    case 0:
      return "NULL";
    case 1:
      return "LOAD";
    case 2:
      return "DYNAMIC";
    case 3:
      return "INTERP";
    case 4:
      return "NOTE";
    case 5:
      return "SHLIB";
    case 6:
      return "PHDR";
    case 7:
      return "TLS";
    case 0x6474e550:
      return "GNU_EH_FRAME";
    case 0x6474e551:
      return "GNU_STACK";
    case 0x6474e552:
      return "GNU_RELRO";
    case 0x6474e553:
      return "GNU_PROPERTY";
    default:
      return "UNKNOWN";
  }
}

const char* elf_segment_at(const ElfSegment* segs, int n, size_t offset) {
  for (int i = 0; i < n; i++) {
    if (offset >= segs[i].offset && offset < segs[i].offset + segs[i].filesz) {
      return elf_segment_type_name(segs[i].type);
    }
  }
  return NULL;
}
