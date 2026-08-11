#include "test.h"

#include "../include/reverse.h"

TEST(reverse_input_kosong) {
  FILE* in = tmpfile();
  FILE* out = tmpfile();
  ASSERT(in && out);
  ASSERT(reverse_hexdump(in, out));
  fclose(in);
  fclose(out);
}

TEST(reverse_single_line) {
  FILE* in = tmpfile();
  FILE* out = tmpfile();
  ASSERT(in && out);

  fprintf(in, "00000000  DE AD BE EF  |....|\n");
  rewind(in);

  ASSERT(reverse_hexdump(in, out));
  rewind(out);

  unsigned char buf[4];
  ASSERT_EQ(fread(buf, 1, 4, out), 4);
  ASSERT_EQ(buf[0], 0xDE);
  ASSERT_EQ(buf[1], 0xAD);
  ASSERT_EQ(buf[2], 0xBE);
  ASSERT_EQ(buf[3], 0xEF);

  fclose(in);
  fclose(out);
}

int main(void) {
  RUN(reverse_single_line);
  RUN(reverse_input_kosong);
  TEST_SUMMARY();
}
