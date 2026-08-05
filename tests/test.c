#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/cli.h"
#include "../include/elf.h"
#include "../include/pattern.h"
#include "../include/reverse.h"

static int g_pass = 0;
static int g_fail = 0;

#define FAIL(msg)                                                              \
  do {                                                                         \
    printf("\n   \033[31mFAIL\033[0m %s:%d : %s\n]", __FILE__, __LINE__, msg); \
    g_fail++;                                                                  \
    return;                                                                    \
  } while (0)

#define ASSERT(cond)          \
  do {                        \
    if (!(cond)) FAIL(#cond); \
  } while (0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp((a), (b)) == 0)

#define TEST(name) static void test_##name(void)
#define RUN(name)                     \
  do {                                \
    printf("   %-36s ", #name);      \
    test_##name();                    \
    printf("\033\[32mPASS\033[0m\n"); \
    g_pass++;                         \
  } while (0)

TEST(pattern_hex_valid) {
  ASSERT(pattern_init_hex("7f454c46"));
  ASSERT(pattern_is_active());
  unsigned char buf[] = {0x7F, 'E', 'L', 'F', 0x00};
  ASSERT(pattern_match_at(buf, sizeof(buf), 0));
}

TEST(reverse_input_kosong) {
  FILE* in = tmpfile();
  FILE* out = tmpfile();
  ASSERT(in && out);
  ASSERT(reverse_hexdump(in, out));
  fclose(in);
  fclose(out);
}

TEST(pattern_hex_with_spaces) {
  ASSERT(pattern_init_hex("7f 45 4c 46"));
  ASSERT(pattern_is_active());
}

TEST(pattern_hex_empty) {
  ASSERT(!pattern_init_hex(""));
  ASSERT(!pattern_is_active());
}

TEST(pattern_ascii_basic) {
  ASSERT(pattern_init_ascii("flag{"));
  ASSERT(pattern_is_active());
  unsigned char buf[] = "xxflag{123}";
  ASSERT(pattern_match_at(buf, sizeof(buf) - 1, 2));
  ASSERT(!pattern_match_at(buf, sizeof(buf) - 1, 0));
}

TEST(pattern_ascii_empty) {
  ASSERT(!pattern_init_ascii(""));
  ASSERT(!pattern_is_active());
}

TEST(pattern_cross_line_highlight) {
  ASSERT(pattern_init_ascii("FLAG{"));

  unsigned char prev[16] = {[14] = 'F', [15] = 'L'};
  unsigned char cur[16] = "AG{SECRET}";
  bool hl[16] = {false};
  pattern_compute_highlights(prev, 16, cur, 10, NULL, 0, hl);
  ASSERT(!hl[0]);

  unsigned char line[16] = {0};
  line[14] = 'F';
  line[15] = 'L';
  pattern_compute_highlights(NULL, 0, line, 16, cur, 10, hl);
  ASSERT(hl[14]);
  ASSERT(!hl[15]);
}

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

int main() {
  RUN(pattern_hex_valid);
  RUN(pattern_hex_with_spaces);
  RUN(pattern_hex_empty);
  RUN(pattern_ascii_basic);
  RUN(pattern_ascii_empty);
  RUN(pattern_cross_line_highlight);
  RUN(cli_basic);
  RUN(cli_offset_hex);
  RUN(cli_offset_dec);
  RUN(cli_length);
  RUN(cli_combined);
  RUN(cli_missing_input);
  RUN(cli_reverse_needs_output);
  RUN(cli_unknown_flag);
  RUN(elf_detect_true);
  RUN(elf_detect_false);
  RUN(elf_machine_x86_64);
  RUN(elf_machine_arm);
  RUN(elf_machine_unknown);
  RUN(elf_field_name_known);
  RUN(elf_field_name_unknown);
  RUN(reverse_single_line);
  RUN(reverse_input_kosong);

  printf("  \033[32m%d passed\033[0m  |  \033[31m%d failed\033[0m\n", g_pass, g_fail);

  return g_fail > 0 ? 1 : 0;
}
