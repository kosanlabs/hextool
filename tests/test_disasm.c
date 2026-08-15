#include "../include/disasm.h"
#include "test.h"

TEST(disasm_nop) {
  uint8_t buf[] = {0x90};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0x1000, out, sizeof(out));
  ASSERT_EQ(n, 1);
  ASSERT_STR_EQ(out, "nop");
}

TEST(disasm_ret) {
  uint8_t buf[] = {0xC3};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 1);
  ASSERT_STR_EQ(out, "ret");
}

TEST(disasm_rex_push_r8) {
  uint8_t buf[] = {0x41, 0x50};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 2);
  ASSERT_STR_EQ(out, "push r8");
}

TEST(disasm_rex_unknown) {
  uint8_t buf[] = {0x41, 0x0C};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 2);
  ASSERT_STR_EQ(out, "db 0x0c");
}

TEST(disasm_call_rel32) {
  uint8_t buf[] = {0xE8, 0x05, 0x00, 0x00, 0x00};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0x1000, out, sizeof(out));
  ASSERT_EQ(n, 5);
  ASSERT_STR_EQ(out, "call 0x100a");
}

TEST(disasm_jmp_rel8) {
  uint8_t buf[] = {0xEB, 0xFE};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0x2000, out, sizeof(out));
  ASSERT_EQ(n, 2);
  ASSERT_STR_EQ(out, "jmp 0x2000");
}

TEST(disasm_mov_modrm) {
  uint8_t buf[] = {0x48, 0x89, 0xE5};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 3);
  ASSERT_STR_EQ(out, "mov rbp, rsp");
}

TEST(disasm_lea_rip) {
  uint8_t buf[] = {0x48, 0x8D, 0x05, 0x00, 0x10, 0x00, 0x00};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 7);
  ASSERT_STR_EQ(out, "lea rax, [rip]");
}

TEST(disasm_syscall) {
  uint8_t buf[] = {0x0F, 0x05};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 2);
  ASSERT_STR_EQ(out, "syscall");
}

TEST(disasm_jcc_rel32) {
  uint8_t buf[] = {0x0F, 0x84, 0x00, 0x10, 0x00, 0x00};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0x1000, out, sizeof(out));
  ASSERT_EQ(n, 6);
  ASSERT_STR_EQ(out, "jz 0x2006");
}

TEST(disasm_sib_rex_b) {
  uint8_t buf[] = {0x41, 0x89, 0x04, 0x24};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 4);
  ASSERT_STR_EQ(out, "mov [r12], eax");
}

TEST(disasm_group_immediate) {
  uint8_t buf[] = {0x83, 0xC4, 0x08};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 3);
  ASSERT_STR_EQ(out, "add esp, 0x8");
}

int main(void) {
  RUN(disasm_nop);
  RUN(disasm_ret);
  RUN(disasm_rex_push_r8);
  RUN(disasm_rex_unknown);
  RUN(disasm_call_rel32);
  RUN(disasm_jmp_rel8);
  RUN(disasm_mov_modrm);
  RUN(disasm_lea_rip);
  RUN(disasm_syscall);
  RUN(disasm_jcc_rel32);
  RUN(disasm_sib_rex_b);
  RUN(disasm_group_immediate);
  TEST_SUMMARY();
}
