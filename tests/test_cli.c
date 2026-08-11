#include "test.h"

#include "../include/cli.h"

TEST(cli_basic) {
  char* argv[] = {"hextool", "/bin/ls"};
  CliArgs args;
  ASSERT(parse_args(2, argv, &args));
  ASSERT_STR_EQ(args.input, "/bin/ls");
  ASSERT_EQ(args.offset, 0);
  ASSERT_EQ(args.length, 0);
  ASSERT(!args.reverse);
}

TEST(cli_offset_hex) {
  char* argv[] = {"hextool", "-o", "0x100", "file"};
  CliArgs args;
  ASSERT(parse_args(4, argv, &args));
  ASSERT_EQ(args.offset, 0x100);
  ASSERT_STR_EQ(args.input, "file");
}

TEST(cli_offset_dec) {
  char* argv[] = {"hextool", "-o", "256", "file"};
  CliArgs args;
  ASSERT(parse_args(4, argv, &args));
  ASSERT_EQ(args.offset, 256);
}

TEST(cli_length) {
  char* argv[] = {"hextool", "-n", "64", "file"};
  CliArgs args;
  ASSERT(parse_args(4, argv, &args));
  ASSERT_EQ(args.length, 64);
}

TEST(cli_combined) {
  char* argv[] = {"hextool", "-o", "0x10", "-n", "32", "file"};
  CliArgs args;
  ASSERT(parse_args(6, argv, &args));
  ASSERT_EQ(args.offset, 0x10);
  ASSERT_EQ(args.length, 32);
}

TEST(cli_missing_input) {
  char* argv[] = {"hextool"};
  CliArgs args;
  ASSERT(!parse_args(1, argv, &args));
}

TEST(cli_reverse_needs_output) {
  char* argv[] = {"hextool", "-r", "input.txt"};
  CliArgs args;
  ASSERT(!parse_args(3, argv, &args));
}

TEST(cli_unknown_flag) {
  char* argv[] = {"hextool", "-x", "file"};
  CliArgs args;
  ASSERT(!parse_args(3, argv, &args));
}

int main(void) {
  RUN(cli_basic);
  RUN(cli_offset_hex);
  RUN(cli_offset_dec);
  RUN(cli_length);
  RUN(cli_combined);
  RUN(cli_missing_input);
  RUN(cli_reverse_needs_output);
  RUN(cli_unknown_flag);
  TEST_SUMMARY();
}
