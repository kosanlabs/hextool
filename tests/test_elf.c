#include "test.h"

#include "../include/elf.h"

TEST(elf_detect_true) {
  FILE* f = tmpfile();
  ASSERT(f != NULL);
  unsigned char elf[] = {0x7F, 'E', 'L', 'F', 0x02, 0x01, 0x01, 0x00};
  fwrite(elf, 1, sizeof(elf), f);
  rewind(f);
  ASSERT(detect_elf(f));
  fclose(f);
}

TEST(elf_detect_false) {
  FILE* f = tmpfile();
  ASSERT(f != NULL);
  fwrite("NOTELF!!", 1, 8, f);
  rewind(f);
  ASSERT(!detect_elf(f));
  fclose(f);
}

TEST(elf_machine_x86_64) {
  ASSERT_STR_EQ(machine_to_arch(0x3E), "x86-64");
}

TEST(elf_machine_arm) {
  ASSERT_STR_EQ(machine_to_arch(0x28), "ARM");
}

TEST(elf_machine_unknown) {
  ASSERT_STR_EQ(machine_to_arch(0x999), "Unknown");
}

TEST(elf_field_name_known) {
  ASSERT_STR_EQ(elf_field_name(0), "e_ident[MAG0-3]");
  ASSERT_STR_EQ(elf_field_name(16), "e_type");
}

TEST(elf_field_name_unknown) {
  ASSERT(elf_field_name(999) == NULL);
}

TEST(elf_segment_type_names) {
  ASSERT_STR_EQ(elf_segment_type_name(0), "NULL");
  ASSERT_STR_EQ(elf_segment_type_name(1), "LOAD");
  ASSERT_STR_EQ(elf_segment_type_name(2), "DYNAMIC");
  ASSERT_STR_EQ(elf_segment_type_name(3), "INTERP");
  ASSERT_STR_EQ(elf_segment_type_name(6), "PHDR");
  ASSERT_STR_EQ(elf_segment_type_name(0x9999), "UNKNOWN");
}

TEST(elf_segment_at_lookup) {
  ElfSegment segs[2];
  segs[0].offset = 0;
  segs[0].filesz = 64;
  segs[0].type = 1;
  segs[1].offset = 128;
  segs[1].filesz = 256;
  segs[1].type = 1;

  ASSERT_STR_EQ(elf_segment_at(segs, 2, 0), "LOAD");
  ASSERT_STR_EQ(elf_segment_at(segs, 2, 32), "LOAD");
  ASSERT(elf_segment_at(segs, 2, 64) == NULL);
  ASSERT_STR_EQ(elf_segment_at(segs, 2, 128), "LOAD");
  ASSERT(elf_segment_at(segs, 2, 500) == NULL);
}

TEST(elf_segments_parse_minimal) {
  FILE* f = tmpfile();
  ASSERT(f != NULL);

  unsigned char hdr[64] = {0};
  hdr[0] = 0x7F;
  hdr[1] = 'E';
  hdr[2] = 'L';
  hdr[3] = 'F';
  hdr[4] = 2;
  hdr[5] = 1;
  hdr[6] = 1;
  hdr[16] = 0x02;
  hdr[17] = 0x00;
  hdr[18] = 0x3E;
  hdr[19] = 0x00;
  hdr[32] = 64;
  hdr[54] = 56;
  hdr[55] = 0;
  hdr[56] = 2;
  hdr[57] = 0;

  unsigned char ph0[56] = {0};
  ph0[0] = 1;
  ph0[4] = 5;
  ph0[8] = 0;
  ph0[32] = 0x00;
  ph0[33] = 0x10;

  unsigned char ph1[56] = {0};
  ph1[0] = 1;
  ph1[4] = 6;
  ph1[8] = 0x00;
  ph1[9] = 0x20;
  ph1[32] = 0x00;
  ph1[33] = 0x01;

  fwrite(hdr, 1, 64, f);
  fwrite(ph0, 1, 56, f);
  fwrite(ph1, 1, 56, f);
  rewind(f);

  ElfSegment segs[MAX_SEGMENTS];
  int n = elf_read_segments(f, segs, MAX_SEGMENTS);
  ASSERT_EQ(n, 2);

  ASSERT_EQ(segs[0].type, 1U);
  ASSERT_EQ(segs[0].offset, 0U);
  ASSERT_EQ(segs[0].filesz, 0x1000U);
  ASSERT_EQ(segs[0].flags, 5U);

  ASSERT_EQ(segs[1].type, 1U);
  ASSERT_EQ(segs[1].offset, 0x2000U);
  ASSERT_EQ(segs[1].filesz, 0x100U);
  ASSERT_EQ(segs[1].flags, 6U);

  fclose(f);
}

int main(void) {
  RUN(elf_detect_true);
  RUN(elf_detect_false);
  RUN(elf_machine_x86_64);
  RUN(elf_machine_arm);
  RUN(elf_machine_unknown);
  RUN(elf_field_name_known);
  RUN(elf_field_name_unknown);
  RUN(elf_segment_type_names);
  RUN(elf_segment_at_lookup);
  RUN(elf_segments_parse_minimal);
  TEST_SUMMARY();
}
