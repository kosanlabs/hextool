#ifndef TEST_H
#define TEST_H

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    printf("   %-36s ", #name);       \
    test_##name();                    \
    printf("\033\[32mPASS\033[0m\n"); \
    g_pass++;                         \
  } while (0)

#define TEST_SUMMARY()                                       \
  do {                                                       \
    printf("  \033[32m%d passed\033[0m  |  \033[31m%d failed\033[0m\n", \
           g_pass, g_fail);                                  \
    return g_fail > 0 ? 1 : 0;                               \
  } while (0)

#endif  // !TEST_H
