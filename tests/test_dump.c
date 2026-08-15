#include "test.h"

#include "../include/binfmt.h"
#include "../include/dump.h"

TEST(dump_file_empty) {
  FILE* f = tmpfile();
  ASSERT(f != NULL);
  DumpStatistik s = {0};
  ASSERT(dump_file(f, &s, 0, 0, false, false));
  ASSERT_EQ(s.total_bytes, 0);
  ASSERT_EQ(s.total_lines, 0);
  fclose(f);
}

TEST(dump_file_counts) {
  FILE* f = tmpfile();
  ASSERT(f != NULL);
  fwrite("A\x00\x80\x01", 1, 4, f);
  rewind(f);
  DumpStatistik s = {0};
  ASSERT(dump_file(f, &s, 0, 0, false, false));
  ASSERT_EQ(s.total_bytes, 4);
  ASSERT_EQ(s.null_count, 1);
  ASSERT_EQ(s.high_count, 1);
  ASSERT_EQ(s.print_count, 1);
  ASSERT_EQ(s.total_lines, 1);
  ASSERT_EQ(s.entropy.count, 1);
  fclose(f);
}

TEST(dump_file_elf_detection) {
  FILE* f = tmpfile();
  ASSERT(f != NULL);
  unsigned char elf[20] = {0};
  elf[0] = 0x7F;
  elf[1] = 'E';
  elf[2] = 'L';
  elf[3] = 'F';
  elf[4] = 0x02;
  elf[18] = 0x3E;
  fwrite(elf, 1, sizeof(elf), f);
  rewind(f);
  DumpStatistik s = {0};
  ASSERT(dump_file(f, &s, 0, 0, false, false));
  ASSERT_EQ(s.format_type, FMT_ELF);
  ASSERT_EQ(s.machine, 0x3E);
  fclose(f);
}

TEST(dump_file_negative_offset) {
  FILE* f = tmpfile();
  ASSERT(f != NULL);
  DumpStatistik s = {0};
  ASSERT(!dump_file(f, &s, -1, 0, false, false));
  fclose(f);
}

TEST(dump_file_max_length) {
  FILE* f = tmpfile();
  ASSERT(f != NULL);
  fwrite("0123456789abcdefghij", 1, 20, f);
  rewind(f);
  DumpStatistik s = {0};
  ASSERT(dump_file(f, &s, 0, 4, false, false));
  ASSERT_EQ(s.total_bytes, 4);
  ASSERT_EQ(s.total_lines, 1);
  fclose(f);
}

TEST(dump_file_offset) {
  FILE* f = tmpfile();
  ASSERT(f != NULL);
  fwrite("ABCDEFGH", 1, 8, f);
  rewind(f);
  DumpStatistik s = {0};
  ASSERT(dump_file(f, &s, 4, 0, false, false));
  ASSERT_EQ(s.total_bytes, 4);
  fclose(f);
}

TEST(dump_file_big_endian) {
  FILE* f = tmpfile();
  ASSERT(f != NULL);
  fwrite("ABCD", 1, 4, f);
  rewind(f);
  DumpStatistik s = {0};
  ASSERT(dump_file(f, &s, 0, 0, true, false));
  ASSERT_EQ(s.total_bytes, 4);
  fclose(f);
}

int main(void) {
  RUN(dump_file_empty);
  RUN(dump_file_counts);
  RUN(dump_file_elf_detection);
  RUN(dump_file_negative_offset);
  RUN(dump_file_max_length);
  RUN(dump_file_offset);
  RUN(dump_file_big_endian);
  TEST_SUMMARY();
}
