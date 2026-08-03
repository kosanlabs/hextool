#include <stdio.h>
#include <stdlib.h>

#include "include/cli.h"
#include "include/dump.h"
#include "include/stats.h"
#include "include/types.h"

int main(int argc, char* argv[]) {
  const char* filename = parse_args(argc, argv);
  if (filename == NULL) {
    return EXIT_FAILURE;
  }

  FILE* file = fopen(filename, "rb");
  if (file == NULL) {
    perror(filename);
    return EXIT_FAILURE;
  }

  DumpStatistik stats = {0};
  if (!dump_file(file, &stats)) {
    fprintf(stderr, "error: gagal membaca file %s\n", filename);
    fclose(file);
    return EXIT_FAILURE;
  }
  fclose(file);

  print_hasil(filename, &stats);
  return 0;
}
