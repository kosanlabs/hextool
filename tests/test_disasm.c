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

TEST(disasm_push_rax) {
  uint8_t buf[] = {0x50};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 1);
  ASSERT_STR_EQ(out, "push rax");
}

TEST(disasm_call_rel32) {
  uint8_t buf[] = {0xE8, 0x05, 0x00, 0x00, 0x00};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0x1000, out, sizeof(out));
  ASSERT_EQ(n, 5);
  ASSERT_STR_EQ(out, "call 0x100a");
}

TEST(disasm_jmp_rel8) {
  uint8_t buf[] = {0xEB, 0xFE};  // jmp to self
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0x2000, out, sizeof(out));
  ASSERT_EQ(n, 2);
  ASSERT_STR_EQ(out, "jmp 0x2000");
}

TEST(disasm_jz_rel8) {
  uint8_t buf[] = {0x74, 0x10};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0x1000, out, sizeof(out));
  ASSERT_EQ(n, 2);
  ASSERT_STR_EQ(out, "jz 0x1012");
}

TEST(disasm_mov_imm32) {
  uint8_t buf[] = {0xB8, 0x78, 0x56, 0x34, 0x12};  // mov eax, 0x12345678
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 5);
  ASSERT_STR_EQ(out, "mov eax, 0x12345678");
}

TEST(disasm_rex_push_r8) {
  uint8_t buf[] = {0x41, 0x50};  // push r8
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 2);
  ASSERT_STR_EQ(out, "push r8");
}

TEST(disasm_syscall) {
  uint8_t buf[] = {0x0F, 0x05};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 2);
  ASSERT_STR_EQ(out, "syscall");
}

TEST(disasm_db_unknown) {
  uint8_t buf[] = {0x0F, 0x38};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 2);
  ASSERT_STR_EQ(out, "db 0x0f, 0x38");
}

TEST(disasm_short_buffer) {
  uint8_t buf[] = {0xE8};
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0, out, sizeof(out));
  ASSERT_EQ(n, 1);
  ASSERT_STR_EQ(out, "db 0xe8");
}

TEST(disasm_jcc_rel32) {
  uint8_t buf[] = {0x0F, 0x84, 0x00, 0x10, 0x00, 0x00};  // jz rel32
  char out[64];
  size_t n = disasm_x86_64(buf, sizeof(buf), 0x1000, out, sizeof(out));
  ASSERT_EQ(n, 6);
  ASSERT_STR_EQ(out, "jz 0x2006");
}

int main(void) {
  RUN(disasm_nop);
  RUN(disasm_ret);
  RUN(disasm_push_rax);
  RUN(disasm_call_rel32);
  RUN(disasm_jmp_rel8);
  RUN(disasm_jz_rel8);
  RUN(disasm_mov_imm32);
  RUN(disasm_rex_push_r8);
  RUN(disasm_syscall);
  RUN(disasm_db_unknown);
  RUN(disasm_short_buffer);
  RUN(disasm_jcc_rel32);
  TEST_SUMMARY();
}
