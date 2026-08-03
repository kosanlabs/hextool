#include <stdio.h>
#include <stdlib.h>

#include "cli.h"
#include "dump.h"
#include "stats.h"
#include "types.h"

int main(int argc, char* argv[]) {
  const char* filename = parse_args(argc, argv);
  if (filename == NULL) return EXIT_FAILURE;

  FILE* file = fopen(filename, "rb");
  if (file == NULL) {
    perror(filename);
    return EXIT_FAILURE;
  }

  DumpStatistik stats = {0};
  dump_file(file, &stats);
  fclose(file);

  print_hasil(filename, &stats);
  return 0;
}
