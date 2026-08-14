#include "../include/binfmt.h"
#include "test.h"

static FILE* tmp_with(const unsigned char* bytes, size_t n) {
  FILE* f = tmpfile();
  if (f) {
    fwrite(bytes, 1, n, f);
    rewind(f);
  }
  return f;
}

TEST(binfmt_detect_elf) {
  unsigned char elf[20] = {0};
  elf[0] = 0x7F;
  elf[1] = 'E';
  elf[2] = 'L';
  elf[3] = 'F';
  elf[4] = 0x02;
  elf[18] = 0x3E;
  FILE* f = tmp_with(elf, sizeof(elf));
  ASSERT(f != NULL);
  BinFmtInfo info = {0};
  ASSERT_EQ(binfmt_detect(f, &info), FMT_ELF);
  ASSERT_EQ(info.machine, 0x3E);
  fclose(f);
}

TEST(binfmt_detect_pe) {
  unsigned char pe[0x90] = {0};
  pe[0] = 'M';
  pe[1] = 'Z';
  pe[0x3C] = 0x80;
  pe[0x80] = 'P';
  pe[0x81] = 'E';
  pe[0x84] = 0x64;
  pe[0x85] = 0x86;
  FILE* f = tmp_with(pe, sizeof(pe));
  ASSERT(f != NULL);
  BinFmtInfo info = {0};
  ASSERT_EQ(binfmt_detect(f, &info), FMT_PE);
  ASSERT_EQ(info.machine, 0x8664);
  ASSERT_EQ(info.pe_offset, 0x80);
  fclose(f);
}

TEST(binfmt_detect_macho) {
  unsigned char macho[8] = {0xCF, 0xFA, 0xED, 0xFE, 0x07, 0x00, 0x00, 0x01};
  FILE* f = tmp_with(macho, sizeof(macho));
  ASSERT(f != NULL);
  BinFmtInfo info = {0};
  ASSERT_EQ(binfmt_detect(f, &info), FMT_MACHO);
  ASSERT_EQ(info.machine, 0x3E);
  fclose(f);
}

TEST(binfmt_detect_unknown) {
  FILE* f = tmp_with((const unsigned char*)"hello world", 11);
  ASSERT(f != NULL);
  BinFmtInfo info = {0};
  ASSERT_EQ(binfmt_detect(f, &info), FMT_UNKNOWN);
  fclose(f);
}

TEST(binfmt_name) {
  ASSERT_STR_EQ(binfmt_name(FMT_ELF), "ELF Executable");
  ASSERT_STR_EQ(binfmt_name(FMT_PE), "PE Executable");
  ASSERT_STR_EQ(binfmt_name(FMT_MACHO), "Mach-O Executable");
  ASSERT_STR_EQ(binfmt_name(FMT_UNKNOWN), "Raw Binary");
}

TEST(binfmt_machine_name) {
  ASSERT_STR_EQ(binfmt_machine_name(FMT_ELF, 0x3E), "x86-64");
  ASSERT_STR_EQ(binfmt_machine_name(FMT_PE, 0x014c), "x86");
  ASSERT_STR_EQ(binfmt_machine_name(FMT_PE, 0x8664), "x86-64");
  ASSERT_STR_EQ(binfmt_machine_name(FMT_PE, 0xdead), "Unknown");
}

TEST(binfmt_field_name_pe) {
  BinFmtInfo info = {0};
  info.type = FMT_PE;
  info.pe_offset = 0x80;
  ASSERT_STR_EQ(binfmt_field_name(FMT_PE, 0, &info), "e_magic");
  ASSERT_STR_EQ(binfmt_field_name(FMT_PE, 0x3C, &info), "e_lfanew");
  ASSERT_STR_EQ(binfmt_field_name(FMT_PE, 0x80, &info), "pe_signature");
  ASSERT_STR_EQ(binfmt_field_name(FMT_PE, 0x84, &info), "pe_machine");
  ASSERT(binfmt_field_name(FMT_PE, 0x10, &info) == NULL);
}

TEST(binfmt_field_name_macho) {
  BinFmtInfo info = {0};
  ASSERT_STR_EQ(binfmt_field_name(FMT_MACHO, 0, &info), "mh_magic");
  ASSERT_STR_EQ(binfmt_field_name(FMT_MACHO, 4, &info), "cputype");
  ASSERT_STR_EQ(binfmt_field_name(FMT_MACHO, 12, &info), "filetype");
  ASSERT(binfmt_field_name(FMT_MACHO, 0x40, &info) == NULL);
}

int main(void) {
  RUN(binfmt_detect_elf);
  RUN(binfmt_detect_pe);
  RUN(binfmt_detect_macho);
  RUN(binfmt_detect_unknown);
  RUN(binfmt_name);
  RUN(binfmt_machine_name);
  RUN(binfmt_field_name_pe);
  RUN(binfmt_field_name_macho);
  TEST_SUMMARY();
}
