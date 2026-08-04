#include <stdio.h>
#include <stdlib.h>

#include "../include/cli.h"
#include "../include/elf.h"
#include "../include/pattern.h"
#include "../include/reverse.h"

static int g_pass = 0;
static int g_fail = 0;

#define FAIL(msg)                                                                     \
  do {                                                                                \
    printf("\n          \033[31mFAIL\033[0m %s:%d : %s\n]", __FILE__, __LINE__, msg); \
    g_fail++;                                                                         \
    return;                                                                           \
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
    printf("   %-36s\n", #name);      \
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

int main() {
  RUN(pattern_hex_valid);

  return 0;
}
