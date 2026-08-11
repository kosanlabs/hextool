#include "test.h"

#include "../include/color.h"
#include "../include/config.h"
#include "../include/format.h"

TEST(entropy_empty) {
  ASSERT_EQ(calc_entropy(NULL, 0), 0.0);
}

TEST(entropy_constant_byte) {
  unsigned char buf[] = {0x41, 0x41, 0x41, 0x41};
  ASSERT_EQ(calc_entropy(buf, 4), 0.0);
}

TEST(entropy_two_values) {
  unsigned char buf[] = {0x00, 0x00, 0xFF, 0xFF};
  ASSERT(fabs(calc_entropy(buf, 4) - 1.0) < 1e-9);
}

TEST(entropy_four_values) {
  unsigned char buf[] = {0x00, 0x11, 0x22, 0x33};
  ASSERT(fabs(calc_entropy(buf, 4) - 2.0) < 1e-9);
}

TEST(entropy_uniform) {
  unsigned char buf[256];
  for (int i = 0; i < 256; i++) buf[i] = (unsigned char)i;
  ASSERT(fabs(calc_entropy(buf, 256) - 8.0) < 1e-9);
}

TEST(entropy_stats_stream) {
  EntropyStats s = {0};
  entropy_update(&s, 1.0);
  entropy_update(&s, 2.0);
  entropy_update(&s, 3.0);
  ASSERT_EQ(s.count, 3);
  ASSERT(fabs(s.mean - 2.0) < 1e-9);
  ASSERT_EQ(s.min, 1.0);
  ASSERT_EQ(s.max, 3.0);
}

TEST(entropy_window_init_reset) {
  EntropyWindow w;
  entropy_window_init(&w);
  ASSERT_EQ(w.len, 0);
  ASSERT_EQ(w.pos, 0);
  ASSERT_EQ(entropy_window_value(&w), 0.0);
}

TEST(entropy_window_basic) {
  EntropyWindow w;
  entropy_window_init(&w);
  unsigned char data[] = {0x00, 0x00, 0xFF, 0xFF};
  entropy_window_push(&w, data, 4);
  double v = entropy_window_value(&w);
  ASSERT(fabs(v - calc_entropy(data, 4)) < 1e-9);
}

TEST(entropy_window_constant) {
  EntropyWindow w;
  entropy_window_init(&w);
  unsigned char data[100];
  memset(data, 'A', sizeof(data));
  entropy_window_push(&w, data, sizeof(data));
  ASSERT_EQ(entropy_window_value(&w), 0.0);
}

TEST(entropy_window_full_uniform) {
  EntropyWindow w;
  entropy_window_init(&w);
  unsigned char data[256];
  for (int i = 0; i < 256; i++) data[i] = (unsigned char)i;
  entropy_window_push(&w, data, sizeof(data));
  ASSERT(fabs(entropy_window_value(&w) - 8.0) < 1e-9);
}

TEST(entropy_window_wrap) {
  EntropyWindow w;
  entropy_window_init(&w);
  unsigned char drop[44];
  memset(drop, 0x00, sizeof(drop));
  unsigned char keep[256];
  memset(keep, 0xFF, sizeof(keep));
  entropy_window_push(&w, drop, sizeof(drop));
  entropy_window_push(&w, keep, sizeof(keep));
  ASSERT_EQ(entropy_window_value(&w), 0.0);
}

TEST(entropy_anomaly_high) {
  EntropyStats s = {0};
  double warm[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0};
  for (size_t i = 0; i < sizeof(warm) / sizeof(warm[0]); i++) {
    entropy_update(&s, warm[i]);
  }
  ASSERT_EQ(s.anomaly_high, 0);
  entropy_update(&s, 8.0);
  ASSERT_EQ(s.anomaly_high, 1);
}

TEST(entropy_anomaly_low) {
  EntropyStats s = {0};
  double warm[] = {7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.0, 7.5};
  for (size_t i = 0; i < sizeof(warm) / sizeof(warm[0]); i++) {
    entropy_update(&s, warm[i]);
  }
  ASSERT_EQ(s.anomaly_low, 0);
  entropy_update(&s, 0.0);
  ASSERT_EQ(s.anomaly_low, 1);
}

TEST(printable_char_basic) {
  ASSERT_EQ(printable_char('A'), 'A');
  ASSERT_EQ(printable_char('\n'), '.');
  ASSERT_EQ(printable_char(0x80), '.');
}

TEST(char_color_classes) {
  ASSERT_STR_EQ(char_color('A', false, false), CLR_PRINT);
  ASSERT_STR_EQ(char_color(0x00, false, false), CLR_NULL);
  ASSERT_STR_EQ(char_color(0x80, false, false), CLR_HIGH);
  ASSERT_STR_EQ(char_color('\x01', false, false), CLR_LOW);
  ASSERT_STR_EQ(char_color(0x41, true, false), CLR_ELF);
  ASSERT_STR_EQ(char_color(0x41, false, true), CLR_MATCH);
}

TEST(color_disable_state) {
  ASSERT(color_is_enabled());
  color_disable();
  ASSERT(!color_is_enabled());
  ASSERT_STR_EQ(char_color('A', false, false), "");
}

int main(void) {
  RUN(entropy_empty);
  RUN(entropy_constant_byte);
  RUN(entropy_two_values);
  RUN(entropy_four_values);
  RUN(entropy_uniform);
  RUN(entropy_stats_stream);
  RUN(entropy_window_init_reset);
  RUN(entropy_window_basic);
  RUN(entropy_window_constant);
  RUN(entropy_window_full_uniform);
  RUN(entropy_window_wrap);
  RUN(entropy_anomaly_high);
  RUN(entropy_anomaly_low);
  RUN(printable_char_basic);
  RUN(char_color_classes);
  RUN(color_disable_state);
  TEST_SUMMARY();
}
