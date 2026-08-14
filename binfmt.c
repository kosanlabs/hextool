#include "include/binfmt.h"

#include <stdint.h>
#include <stdio.h>

#include "include/config.h"
#include "include/elf.h"

static int detect_elf_internal(FILE* file, unsigned short* machine) {
  unsigned char magic[ELF_MAGIC_SIZE];
  size_t n = fread(magic, 1, ELF_MAGIC_SIZE, file);

  if (n != ELF_MAGIC_SIZE || magic[0] != 0x7F || magic[1] != 'E' || magic[2] != 'L' ||
      magic[3] != 'F') {
    return 0;
  }

  unsigned char buf[ELF_MAGIC_SIZE];
  if (fseek(file, ELF_MACHINE_OFFSET, SEEK_SET) != 0) {
    return 0;
  }

  if (fread(buf, 1, ELF_MACHINE_SIZE, file) != ELF_MACHINE_SIZE) {
    return 0;
  }

  *machine = (unsigned short)(buf[0] | (buf[1] << 8));
  return FMT_ELF;
}

static int detect_pe_internal(FILE* file, unsigned short* machine, uint32_t* pe_offset) {
  unsigned char mz[2];
  if (fseek(file, 0, SEEK_SET) != 0) {
    return 0;
  }
  if (fread(mz, 1, 2, file) != 2) {
    return 0;
  }
  if (mz[0] != 'M' || mz[1] != 'Z') {
    return 0;
  }

  unsigned char off[4];
  if (fseek(file, 0x3C, SEEK_SET) != 0) {
    return 0;
  }
  if (fread(off, 1, 4, file) != 4) {
    return 0;
  }
  uint32_t pe = (uint32_t)off[0] | ((uint32_t)off[1] << 8) | ((uint32_t)off[2] << 16) |
                ((uint32_t)off[3] << 24);

  if (fseek(file, pe, SEEK_SET) != 0) {
    return 0;
  }
  unsigned char sig[4];
  if (fread(sig, 1, 4, file) != 4) {
    return 0;
  }
  if (sig[0] != 'P' || sig[1] != 'E' || sig[2] != 0 || sig[3] != 0) {
    return 0;
  }

  unsigned char m[2];
  if (fread(m, 1, 2, file) != 2) {
    return 0;
  }
  *machine = (unsigned short)(m[0] | (m[1] << 8));
  *pe_offset = pe;
  return FMT_PE;
}

static int detect_macho_internal(FILE* file, unsigned short* machine) {
  unsigned char magic[4];
  if (fseek(file, 0, SEEK_SET) != 0) {
    return 0;
  }
  if (fread(magic, 1, 4, file) != 4) {
    return 0;
  }

  uint32_t mag = (uint32_t)magic[0] | ((uint32_t)magic[1] << 8) | ((uint32_t)magic[2] << 16) |
                 ((uint32_t)magic[3] << 24);
  bool be = false;

  if (mag == 0xfeedface) {
  } else if (mag == 0xfeedfacf) {
  } else if (mag == 0xcefaedfe) {
    be = true;
  } else if (mag == 0xcffaedfe) {
    be = true;
  } else if (mag == 0xcafebabe || mag == 0xbebafeca) {
    *machine = 0;
    return FMT_MACHO;
  } else {
    return 0;
  }

  unsigned char cpu[4];
  if (fread(cpu, 1, 4, file) != 4) {
    return 0;
  }
  uint32_t cputype =
      be ? ((uint32_t)cpu[0] << 24 | (uint32_t)cpu[1] << 16 | (uint32_t)cpu[2] << 8 | cpu[3])
         : ((uint32_t)cpu[3] << 24 | (uint32_t)cpu[2] << 16 | (uint32_t)cpu[1] << 8 | cpu[0]);

  switch (cputype) {
    case 0x00000007:
      *machine = 0x03;
      break;  // x86
    case 0x01000007:
      *machine = 0x3E;
      break;  // x86-64
    case 0x0000000C:
      *machine = 0x28;
      break;  // ARM
    case 0x0100000C:
      *machine = 0xB7;
      break;  // ARM64
    case 0x00000012:
      *machine = 0x14;
      break;  // PowerPC
    case 0x01000012:
      *machine = 0x15;
      break;  // PowerPC64
    default:
      *machine = 0;
      break;
  }
  return FMT_MACHO;
}

int binfmt_detect(FILE* file, BinFmtInfo* out) {
  out->type = FMT_UNKNOWN;
  out->machine = 0;
  out->pe_offset = 0;

  int type = detect_elf_internal(file, &out->machine);
  if (type) {
    out->type = type;
    rewind(file);
    return type;
  }

  type = detect_pe_internal(file, &out->machine, &out->pe_offset);
  if (type) {
    out->type = type;
    rewind(file);
    return type;
  }

  type = detect_macho_internal(file, &out->machine);
  if (type) {
    out->type = type;
    rewind(file);
    return type;
  }

  rewind(file);
  return FMT_UNKNOWN;
}

const char* binfmt_name(int type) {
  switch (type) {
    case FMT_ELF:
      return "ELF Executable";
    case FMT_PE:
      return "PE Executable";
    case FMT_MACHO:
      return "Mach-O Executable";
    default:
      return "Raw Binary";
  }
}

const char* binfmt_machine_name(int type, unsigned short machine) {
  if (type == FMT_ELF || type == FMT_MACHO) {
    return machine_to_arch(machine);
  }
  if (type == FMT_PE) {
    switch (machine) {
      case 0x014c:
        return "x86";
      case 0x8664:
        return "x86-64";
      case 0x01c0:
        return "ARM";
      case 0xaa64:
        return "ARM64";
      case 0x0200:
        return "IA-64";
      default:
        return "Unknown";
    }
  }
  return "Unknown";
}

const char* binfmt_field_name(int type, size_t offset, const BinFmtInfo* info) {
  if (type == FMT_ELF) {
    return elf_field_name(offset);
  }
  if (type == FMT_PE) {
    if (offset == 0) {
      return "e_magic";
    }
    if (offset == 0x3C) {
      return "e_lfanew";
    }
    if (info->pe_offset > 0) {
      if (offset == info->pe_offset) {
        return "pe_signature";
      }
      if (offset == info->pe_offset + 4) {
        return "pe_machine";
      }
    }
    return NULL;
  }
  if (type == FMT_MACHO) {
    if (offset < 4) {
      return "mh_magic";
    }
    if (offset < 8) {
      return "cputype";
    }
    if (offset < 12) {
      return "cpusubtype";
    }
    if (offset == 12) {
      return "filetype";
    }
    if (offset == 16) {
      return "ncmds";
    }
    if (offset == 20) {
      return "sizeofcmds";
    }
    if (offset == 24) {
      return "flags";
    }
    if (offset == 28) {
      return "reserved";
    }
    return NULL;
  }
  return NULL;
}
