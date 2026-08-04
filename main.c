#include <stdio.h>
#include <stdlib.h>

#include "include/cli.h"
#include "include/dump.h"
#include "include/reverse.h"
#include "include/stats.h"
#include "include/types.h"

int main(int argc, char* argv[]) {
  CliArgs args;
  if (!parse_args(argc, argv, &args)) {
    return EXIT_FAILURE;
  }

  if (args.offset > 0 || args.length > 0) {
    if (args.reverse) {
      fprintf(stderr, "error: -o/-n gak kompatible dengan mode reverse\n");
      return EXIT_FAILURE;
    }
  }

  if (args.reverse) {
    FILE* in = fopen(args.input, "r");
    if (in == NULL) {
      perror(args.input);
      return EXIT_FAILURE;
    }
    FILE* out = fopen(args.output, "wb");
    if (out == NULL) {
      perror(args.output);
      fclose(in);
      return EXIT_FAILURE;
    }
    if (!reverse_hexdump(in, out)) {
      fprintf(stderr, "error: reverse hexdump gagal\n");
      fclose(in);
      fclose(out);
      return EXIT_FAILURE;
    }
    fclose(in);
    fclose(out);
    printf("Reverse selesai: %s -> %s\n", args.input, args.output);
    return EXIT_SUCCESS;
  }

  FILE* file = fopen(args.input, "rb");
  if (file == NULL) {
    perror(args.input);
    return EXIT_FAILURE;
  }

  if (args.offset > 0 || args.length > 0) {
    if (args.reverse) {
      fprintf(stderr, "error: -o/-n tidak kompatible dengan mode reverse\n");
      return EXIT_FAILURE;
    }
  }

  DumpStatistik stats = {0};
  if (!dump_file(file, &stats, args.offset, args.length)) {
    fprintf(stderr, "error: gagal membaca file %s\n", args.input);
    fclose(file);
    return EXIT_FAILURE;
  }
  fclose(file);

  if (stats.total_lines == 0) {
    fprintf(stderr, "warning: file kosong, tidak ada data untuk di dump\n");
  }

  print_hasil(args.input, &stats);
  return 0;
}
