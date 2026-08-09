#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  const char* input;
  const char* output;
  const char* diff_file;
  bool reverse;
  long offset;
  unsigned long long length;
  bool help;
  bool version;
  bool big_endian;
} CliArgs;

bool parse_args(int argc, char* argv[], CliArgs* out);
void print_usage(const char* prog);

#endif  // !CLI_H
