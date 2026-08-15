#include "include/disasm.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const char* regs_b[16] = {"al",  "cl",  "dl",   "bl",   "spl",  "bpl",  "sil",  "dil",
                                 "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"};
static const char* regs_d[16] = {"eax", "ecx", "edx",  "ebx",  "esp",  "ebp",  "esi",  "edi",
                                 "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"};
static const char* regs_q[16] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
                                 "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"};

typedef struct {
  const char* name;
  uint8_t opcode;
  uint8_t has_modrm;
  uint8_t imm_size;
} OpInfo;

static const OpInfo op_table[] = {
    {"nop", 0x90, 0, 0},   {"push", 0x50, 0, 0},  {"push", 0x51, 0, 0},  {"push", 0x52, 0, 0},
    {"push", 0x53, 0, 0},  {"push", 0x54, 0, 0},  {"push", 0x55, 0, 0},  {"push", 0x56, 0, 0},
    {"push", 0x57, 0, 0},  {"pop", 0x58, 0, 0},   {"pop", 0x59, 0, 0},   {"pop", 0x5A, 0, 0},
    {"pop", 0x5B, 0, 0},   {"pop", 0x5C, 0, 0},   {"pop", 0x5D, 0, 0},   {"pop", 0x5E, 0, 0},
    {"pop", 0x5F, 0, 0},   {"push", 0x68, 0, 4},  {"push", 0x6A, 0, 1},  {"jo", 0x70, 0, 1},
    {"jno", 0x71, 0, 1},   {"jb", 0x72, 0, 1},    {"jnb", 0x73, 0, 1},   {"jz", 0x74, 0, 1},
    {"jnz", 0x75, 0, 1},   {"jbe", 0x76, 0, 1},   {"ja", 0x77, 0, 1},    {"js", 0x78, 0, 1},
    {"jns", 0x79, 0, 1},   {"jp", 0x7A, 0, 1},    {"jnp", 0x7B, 0, 1},   {"jl", 0x7C, 0, 1},
    {"jge", 0x7D, 0, 1},   {"jle", 0x7E, 0, 1},   {"jg", 0x7F, 0, 1},    {"add", 0x80, 1, 1},
    {"add", 0x81, 1, 4},   {"add", 0x83, 1, 1},   {"test", 0x84, 1, 0},  {"test", 0x85, 1, 0},
    {"add", 0x01, 1, 0},   {"add", 0x03, 1, 0},   {"or", 0x09, 1, 0},    {"or", 0x0B, 1, 0},
    {"adc", 0x11, 1, 0},   {"adc", 0x13, 1, 0},   {"sbb", 0x19, 1, 0},   {"sbb", 0x1B, 1, 0},
    {"and", 0x21, 1, 0},   {"and", 0x23, 1, 0},   {"sub", 0x29, 1, 0},   {"sub", 0x2B, 1, 0},
    {"xor", 0x31, 1, 0},   {"xor", 0x33, 1, 0},   {"cmp", 0x39, 1, 0},   {"cmp", 0x3B, 1, 0},
    {"xchg", 0x86, 1, 0},  {"xchg", 0x87, 1, 0},  {"mov", 0x88, 1, 0},   {"mov", 0x89, 1, 0},
    {"mov", 0x8A, 1, 0},   {"mov", 0x8B, 1, 0},   {"mov", 0x8C, 1, 0},   {"lea", 0x8D, 1, 0},
    {"mov", 0x8E, 1, 0},   {"nop", 0x8F, 0, 0},   {"xchg", 0x90, 0, 0},  {"cdqe", 0x98, 0, 0},
    {"cqo", 0x99, 0, 0},   {"mov", 0xA0, 0, 0},   {"mov", 0xA1, 0, 0},   {"mov", 0xA2, 0, 0},
    {"mov", 0xA3, 0, 0},   {"movsb", 0xA4, 0, 0}, {"movsw", 0xA5, 0, 0}, {"cmpsb", 0xA6, 0, 0},
    {"cmpsw", 0xA7, 0, 0}, {"test", 0xA8, 0, 1},  {"test", 0xA9, 0, 4},  {"stosb", 0xAA, 0, 0},
    {"stosw", 0xAB, 0, 0}, {"lodsb", 0xAC, 0, 0}, {"lodsw", 0xAD, 0, 0}, {"scasb", 0xAE, 0, 0},
    {"scasw", 0xAF, 0, 0}, {"mov", 0xB0, 0, 1},   {"mov", 0xB1, 0, 1},   {"mov", 0xB2, 0, 1},
    {"mov", 0xB3, 0, 1},   {"mov", 0xB4, 0, 1},   {"mov", 0xB5, 0, 1},   {"mov", 0xB6, 0, 1},
    {"mov", 0xB7, 0, 1},   {"mov", 0xB8, 0, 4},   {"mov", 0xB9, 0, 4},   {"mov", 0xBA, 0, 4},
    {"mov", 0xBB, 0, 4},   {"mov", 0xBC, 0, 4},   {"mov", 0xBD, 0, 4},   {"mov", 0xBE, 0, 4},
    {"mov", 0xBF, 0, 4},   {"ret", 0xC3, 0, 0},   {"leave", 0xC9, 0, 0}, {"mov", 0xC7, 1, 4},
    {"call", 0xE8, 0, 4},  {"jmp", 0xE9, 0, 4},   {"jmp", 0xEB, 0, 1},   {"lock", 0xF0, 0, 0},
    {"rep", 0xF3, 0, 0},   {"hlt", 0xF4, 0, 0},   {"cmc", 0xF5, 0, 0},   {"test", 0xF6, 1, 1},
    {"test", 0xF7, 1, 4},  {"clc", 0xF8, 0, 0},   {"stc", 0xF9, 0, 0},   {"cli", 0xFA, 0, 0},
    {"sti", 0xFB, 0, 0},   {"cld", 0xFC, 0, 0},   {"std", 0xFD, 0, 0},   {"inc", 0xFE, 1, 0},
    {"inc", 0xFF, 1, 0},
};

static const size_t op_table_count = sizeof(op_table) / sizeof(op_table[0]);

static bool is_byte_op(uint8_t opcode) {
  return opcode == 0x80 || opcode == 0x84 || opcode == 0x86 || opcode == 0x88 || opcode == 0x8A ||
         opcode == 0x8C || opcode == 0x8E || opcode == 0xC6 || opcode == 0xF6 || opcode == 0xFE;
}

static const OpInfo* lookup_op(uint8_t opcode) {
  for (size_t i = 0; i < op_table_count; i++) {
    if (op_table[i].opcode == opcode) return &op_table[i];
  }
  return NULL;
}

static uint8_t modrm_len(uint8_t modrm, const uint8_t* after_modrm, size_t remain) {
  uint8_t mod = (modrm >> 6) & 3;
  uint8_t rm = modrm & 7;

  if (mod == 3) return 1;

  uint8_t extra = 1;

  if (rm == 4) {
    if (remain < 1) return 1;
    extra++;
    uint8_t sib = after_modrm[0];
    uint8_t base = sib & 7;
    if (mod == 0 && base == 5) {
      extra += 4;
    }
  } else if (mod == 0 && rm == 5) {
    extra += 4;
  } else if (mod == 1) {
    extra += 1;
  } else if (mod == 2) {
    extra += 4;
  }

  return extra;
}

static void format_rm(char* out, size_t n, uint8_t modrm, uint8_t rex, const char* const* regs,
                      const uint8_t* extra, size_t extra_len) {
  uint8_t mod = modrm >> 6;
  uint8_t rm = (modrm & 7) + ((rex & 1) ? 8 : 0);

  if (mod == 3) {
    snprintf(out, n, "%s", regs[rm]);
    return;
  }

  if (mod == 0 && (modrm & 7) == 5) {
    snprintf(out, n, "[rip]");
    return;
  }

  if ((modrm & 7) == 4) {
    if (extra_len < 1) {
      snprintf(out, n, "[?]");
      return;
    }
    uint8_t sib = extra[0];
    uint8_t base = (sib & 7) + ((rex & 1) ? 8 : 0);
    const char* base_name = (base < 16) ? regs_q[base] : "?";
    snprintf(out, n, "[%s]", base_name);
    return;
  }

  const char* base = (rm < 16) ? regs_q[rm] : "?";
  snprintf(out, n, "[%s]", base);
}

size_t disasm_x86_64(const uint8_t* buf, size_t len, size_t offset, char* out, size_t out_len) {
  if (len == 0 || out_len < 4) return 0;

  size_t pos = 0;
  uint8_t rex = 0;

  while (pos < len && (buf[pos] & 0xF0) == 0x40) {
    rex = buf[pos];
    pos++;
  }

  if (pos >= len) {
    snprintf(out, out_len, "db 0x%02x", buf[0]);
    return 1;
  }

  uint8_t opcode = buf[pos];
  const OpInfo* op = lookup_op(opcode);

  if (opcode == 0x0F && pos + 1 < len) {
    uint8_t op2 = buf[pos + 1];
    const char* name = NULL;
    uint8_t imm = 0;

    if (op2 == 0x05) {
      name = "syscall";
    } else if (op2 == 0x34) {
      name = "sysenter";
    } else if (op2 == 0x35) {
      name = "sysexit";
    } else if (op2 == 0x82) {
      name = "jo";
      imm = 4;
    } else if (op2 == 0x83) {
      name = "jno";
      imm = 4;
    } else if (op2 == 0x84) {
      name = "jz";
      imm = 4;
    } else if (op2 == 0x85) {
      name = "jnz";
      imm = 4;
    } else if (op2 == 0x86) {
      name = "jbe";
      imm = 4;
    } else if (op2 == 0x87) {
      name = "ja";
      imm = 4;
    } else if (op2 == 0x88) {
      name = "js";
      imm = 4;
    } else if (op2 == 0x89) {
      name = "jns";
      imm = 4;
    } else if (op2 == 0x8A) {
      name = "jp";
      imm = 4;
    } else if (op2 == 0x8B) {
      name = "jnp";
      imm = 4;
    } else if (op2 == 0x8C) {
      name = "jl";
      imm = 4;
    } else if (op2 == 0x8D) {
      name = "jge";
      imm = 4;
    } else if (op2 == 0x8E) {
      name = "jle";
      imm = 4;
    } else if (op2 == 0x8F) {
      name = "jg";
      imm = 4;
    } else if (op2 == 0xAF) {
      name = "imul";
    } else if (op2 == 0xB6 || op2 == 0xB7) {
      name = "movzx";
    } else if (op2 == 0xBE || op2 == 0xBF) {
      name = "movsx";
    } else if ((op2 & 0xF0) == 0x80) {
      name = "jo";
      imm = 4;
    }

    if (name) {
      size_t total = pos + 2 + imm;
      if (imm == 4 && pos + 6 <= len) {
        int32_t rel = (int32_t)(buf[pos + 2] | (buf[pos + 3] << 8) | (buf[pos + 4] << 16) |
                                (buf[pos + 5] << 24));
        snprintf(out, out_len, "%s 0x%zx", name, offset + total + rel);
        return total;
      }
      snprintf(out, out_len, "%s", name);
      return total;
    }

    snprintf(out, out_len, "db 0x0f, 0x%02x", op2);
    return pos + 2;
  }

  if (opcode == 0x66 || opcode == 0x67) {
    if (pos + 1 < len) {
      char inner[64];
      size_t inner_len =
          disasm_x86_64(buf + pos + 1, len - pos - 1, offset + pos + 1, inner, sizeof(inner));
      snprintf(out, out_len, "%s", inner);
      return pos + 1 + inner_len;
    }
    snprintf(out, out_len, "db 0x%02x", opcode);
    return 1;
  }

  if (!op) {
    snprintf(out, out_len, "db 0x%02x", buf[pos]);
    return pos + 1;
  }

  if ((opcode >= 0x50 && opcode <= 0x57) || (opcode >= 0x58 && opcode <= 0x5F)) {
    uint8_t reg = (opcode & 7) + ((rex & 1) ? 8 : 0);
    snprintf(out, out_len, "%s %s", op->name, regs_q[reg]);
    return pos + 1;
  }

  if (opcode >= 0xB0 && opcode <= 0xBF) {
    uint8_t reg = (opcode & 7) + ((rex & 1) ? 8 : 0);
    bool w = (rex & 0x08) != 0;
    const char* const* regs = (opcode < 0xB8) ? regs_b : (w ? regs_q : regs_d);

    if (opcode < 0xB8) {
      if (pos + 2 <= len) {
        snprintf(out, out_len, "mov %s, 0x%02x", regs[reg], buf[pos + 1]);
        return pos + 2;
      }
    } else {
      if (pos + 5 <= len) {
        uint32_t imm = (uint32_t)(buf[pos + 1] | (buf[pos + 2] << 8) | (buf[pos + 3] << 16) |
                                  (buf[pos + 4] << 24));
        snprintf(out, out_len, "mov %s, 0x%x", regs[reg], imm);
        return pos + 5;
      }
    }
    snprintf(out, out_len, "db 0x%02x", opcode);
    return pos + 1;
  }

  size_t total = pos + 1;

  if (op->has_modrm) {
    if (total >= len) {
      snprintf(out, out_len, "db 0x%02x", opcode);
      return pos + 1;
    }
    uint8_t modrm = buf[total];
    uint8_t mlen = modrm_len(modrm, buf + total + 1, len - total - 1);
    total += mlen;
  }

  total += op->imm_size;

  if (total > len) {
    snprintf(out, out_len, "db 0x%02x", buf[pos]);
    return pos + 1;
  }

  if (op->has_modrm && total <= len) {
    uint8_t modrm = buf[pos + 1];
    uint8_t reg = ((modrm >> 3) & 7) + ((rex & 0x04) ? 8 : 0);
    bool w = (rex & 0x08) != 0;
    const char* const* regs = is_byte_op(opcode) ? regs_b : (w ? regs_q : regs_d);
    char rm_s[32], reg_s[32];
    format_rm(rm_s, sizeof(rm_s), modrm, rex, regs, buf + pos + 2, len - pos - 2);
    snprintf(reg_s, sizeof(reg_s), "%s", regs[reg]);

    switch (opcode) {
      case 0x01:
      case 0x03:
        snprintf(out, out_len, "add %s, %s", rm_s, reg_s);
        return total;
      case 0x09:
      case 0x0B:
        snprintf(out, out_len, "or %s, %s", rm_s, reg_s);
        return total;
      case 0x11:
      case 0x13:
        snprintf(out, out_len, "adc %s, %s", rm_s, reg_s);
        return total;
      case 0x19:
      case 0x1B:
        snprintf(out, out_len, "sbb %s, %s", rm_s, reg_s);
        return total;
      case 0x21:
      case 0x23:
        snprintf(out, out_len, "and %s, %s", rm_s, reg_s);
        return total;
      case 0x29:
      case 0x2B:
        snprintf(out, out_len, "sub %s, %s", rm_s, reg_s);
        return total;
      case 0x31:
      case 0x33:
        snprintf(out, out_len, "xor %s, %s", rm_s, reg_s);
        return total;
      case 0x39:
      case 0x3B:
        snprintf(out, out_len, "cmp %s, %s", rm_s, reg_s);
        return total;
      case 0x84:
      case 0x85:
        snprintf(out, out_len, "test %s, %s", rm_s, reg_s);
        return total;
      case 0x86:
      case 0x87:
        snprintf(out, out_len, "xchg %s, %s", rm_s, reg_s);
        return total;
      case 0x88:
      case 0x89:
        snprintf(out, out_len, "mov %s, %s", rm_s, reg_s);
        return total;
      case 0x8A:
      case 0x8B:
        snprintf(out, out_len, "mov %s, %s", reg_s, rm_s);
        return total;
      case 0x8D:
        snprintf(out, out_len, "lea %s, %s", reg_s, rm_s);
        return total;
      default:
        break;
    }

    // Group opcodes
    if (opcode == 0x80 || opcode == 0x81 || opcode == 0x83 || opcode == 0xC6 || opcode == 0xC7 ||
        opcode == 0xF6 || opcode == 0xF7) {
      static const char* group_names[8] = {"add", "or", "adc", "sbb", "and", "sub", "xor", "cmp"};
      uint8_t digit = (modrm >> 3) & 7;
      const char* gname = (opcode == 0xC6 || opcode == 0xC7)   ? "mov"
                          : (opcode == 0xF6 || opcode == 0xF7) ? (digit == 0 ? "test" : NULL)
                                                               : group_names[digit];

      if (gname) {
        size_t imm_pos = pos + 1 + modrm_len(modrm, buf + pos + 2, len - pos - 2);
        uint64_t imm = 0;
        if (op->imm_size == 1 && imm_pos < len) {
          imm = (uint64_t)(int64_t)(int8_t)buf[imm_pos];
        } else if (op->imm_size == 4 && imm_pos + 4 <= len) {
          int32_t v = (int32_t)(buf[imm_pos] | (buf[imm_pos + 1] << 8) | (buf[imm_pos + 2] << 16) |
                                (buf[imm_pos + 3] << 24));
          imm = w ? (uint64_t)(int64_t)v : (uint64_t)v;
        } else {
          gname = NULL;
        }

        if (gname) {
          snprintf(out, out_len, "%s %s, 0x%llx", gname, rm_s, (unsigned long long)imm);
          return total;
        }
      }
    }
  }

  if ((opcode == 0xE8 || opcode == 0xE9) && op->imm_size == 4 && pos + 5 <= len) {
    int32_t rel =
        (int32_t)(buf[pos + 1] | (buf[pos + 2] << 8) | (buf[pos + 3] << 16) | (buf[pos + 4] << 24));
    snprintf(out, out_len, "%s 0x%zx", op->name, offset + total + rel);
    return total;
  }
  if (opcode == 0xEB && pos + 2 <= len) {
    int8_t rel = (int8_t)buf[pos + 1];
    snprintf(out, out_len, "%s 0x%zx", op->name, offset + total + rel);
    return total;
  }
  if (opcode >= 0x70 && opcode <= 0x7F && pos + 2 <= len) {
    int8_t rel = (int8_t)buf[pos + 1];
    snprintf(out, out_len, "%s 0x%zx", op->name, offset + total + rel);
    return total;
  }

  snprintf(out, out_len, "%s", op->name);
  return total;
}
