#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  const char* input;
  const char* output;
  bool reverse;
  long offset;
  size_t length;
} CliArgs;

bool parse_args(int argc, char* argv[], CliArgs* out);

#endif // !CLI_H
