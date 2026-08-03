#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

typedef struct {
  const char* input;
  const char* output;
  bool reverse;
} CliArgs;

bool parse_args(int argc, char* argv[], CliArgs* out);

#endif // !CLI_H
