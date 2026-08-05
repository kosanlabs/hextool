#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

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

  if (args.help) {
    print_usage(argv[0]);
    return EXIT_SUCCESS;
  }

  if (args.version) {
    printf("hextool v1.0\n");
    return EXIT_SUCCESS;
  }

  if ((args.offset > 0 || args.length > 0) && args.reverse) {
    fprintf(stderr, "error: -o/-n tidak kompatibel dengan mode reverse\n");
    return EXIT_FAILURE;
  }

  if (args.reverse) {
    FILE* in = fopen(args.input, "rb");
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

  if (strcmp(args.input, "-") == 0) {
    fprintf(stderr, "error: membaca dari stdin belum didukung\n");
    return EXIT_FAILURE;
  }

  struct stat st;
  if (stat(args.input, &st) == 0 && S_ISDIR(st.st_mode)) {
    fprintf(stderr, "error: %s adalah direktori\n", args.input);
    return EXIT_FAILURE;
  }

  FILE* file = fopen(args.input, "rb");
  if (file == NULL) {
    perror(args.input);
    return EXIT_FAILURE;
  }

  setvbuf(stdout, NULL, _IOFBF, 65536);

  if (args.offset > 0) {
    if (fseek(file, 0, SEEK_END) != 0) {
      perror("fseek");
      fclose(file);
      return EXIT_FAILURE;
    }
    long file_size = ftell(file);
    if (file_size >= 0 && args.offset >= file_size) {
      fprintf(stderr, "error: offset (0x%lx) melebihi ukuran file (%ld byte)\n", args.offset,
              file_size);
      fclose(file);
      return EXIT_FAILURE;
    }
    rewind(file);
  }

  DumpStatistik stats = {0};
  if (!dump_file(file, &stats, args.offset, args.length)) {
    fprintf(stderr, "error: gagal membaca file %s\n", args.input);
    fclose(file);
    return EXIT_FAILURE;
  }
  fclose(file);

  if (stats.total_lines == 0) {
    fprintf(stderr, "warning: file kosong, tidak ada data untuk di-dump\n");
  }

  print_hasil(args.input, &stats);
  return 0;
}
