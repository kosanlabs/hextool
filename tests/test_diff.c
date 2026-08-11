#include "test.h"

#include "../include/diff.h"

static void write_file(const char* path, const char* data, size_t n) {
  FILE* f = fopen(path, "wb");
  ASSERT(f != NULL);
  fwrite(data, 1, n, f);
  fclose(f);
}

TEST(diff_identical) {
  char p1[] = "/tmp/hextool_diff_XXXXXX";
  char p2[] = "/tmp/hextool_diff_XXXXXX";
  int fd1 = mkstemp(p1);
  int fd2 = mkstemp(p2);
  ASSERT(fd1 >= 0 && fd2 >= 0);
  close(fd1);
  close(fd2);
  write_file(p1, "hello", 5);
  write_file(p2, "hello", 5);
  ASSERT_EQ(diff_files(p1, p2), 0);
  unlink(p1);
  unlink(p2);
}

TEST(diff_differs) {
  char p1[] = "/tmp/hextool_diff_XXXXXX";
  char p2[] = "/tmp/hextool_diff_XXXXXX";
  int fd1 = mkstemp(p1);
  int fd2 = mkstemp(p2);
  ASSERT(fd1 >= 0 && fd2 >= 0);
  close(fd1);
  close(fd2);
  write_file(p1, "hello", 5);
  write_file(p2, "world", 5);
  ASSERT_EQ(diff_files(p1, p2), 1);
  unlink(p1);
  unlink(p2);
}

TEST(diff_length_mismatch) {
  char p1[] = "/tmp/hextool_diff_XXXXXX";
  char p2[] = "/tmp/hextool_diff_XXXXXX";
  int fd1 = mkstemp(p1);
  int fd2 = mkstemp(p2);
  ASSERT(fd1 >= 0 && fd2 >= 0);
  close(fd1);
  close(fd2);
  write_file(p1, "", 0);
  write_file(p2, "\x00", 1);
  ASSERT_EQ(diff_files(p1, p2), 1);
  unlink(p1);
  unlink(p2);
}

TEST(diff_missing_file) {
  ASSERT_EQ(diff_files("/nonexistent_a_xyz", "/nonexistent_b_xyz"), -1);
}

int main(void) {
  RUN(diff_identical);
  RUN(diff_differs);
  RUN(diff_length_mismatch);
  RUN(diff_missing_file);
  TEST_SUMMARY();
}
