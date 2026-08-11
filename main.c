#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "include/cli.h"
#include "include/diff.h"
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
    printf("hextool v1.1.0\n");
    return EXIT_SUCCESS;
  }

  if ((args.offset > 0 || args.length > 0) && args.reverse) {
    fprintf(stderr, "error: -o/-n are not compatible with reverse mode\n");
    return EXIT_FAILURE;
  }

  signal(SIGPIPE, SIG_IGN);

  if (args.diff_file) {
    int rc = diff_files(args.diff_file, args.input);
    return rc < 0 ? 2 : rc;
  }

  if (args.reverse) {
    int rc = EXIT_FAILURE;
    FILE* in = NULL;
    FILE* out = NULL;

    in = fopen(args.input, "rb");
    if (in == NULL) {
      perror(args.input);
      goto reverse_cleanup;
    }

    struct stat st_out;
    if (stat(args.output, &st_out) == 0) {
      if (!S_ISREG(st_out.st_mode)) {
        fprintf(stderr, "error: %s exists and is not a regular file\n", args.output);
        goto reverse_cleanup;
      }
    }

    out = fopen(args.output, "wb");
    if (out == NULL) {
      perror(args.output);
      goto reverse_cleanup;
    }

    if (!reverse_hexdump(in, out)) {
      fprintf(stderr, "error: reverse hexdump failed\n");
      goto reverse_cleanup;
    }

    printf("Reverse done: %s -> %s\n", args.input, args.output);
    rc = EXIT_SUCCESS;

  reverse_cleanup:
    if (in) fclose(in);
    if (out && fclose(out) != 0) {
      perror(args.output);
      rc = EXIT_FAILURE;
    }
    return rc;
  }

  if (strcmp(args.input, "-") == 0) {
    fprintf(stderr, "error: reading from stdin is not supported\n");
    return EXIT_FAILURE;
  }

  FILE* file = fopen(args.input, "rb");
  if (file == NULL) {
    perror(args.input);
    return EXIT_FAILURE;
  }

  struct stat st;
  if (fstat(fileno(file), &st) != 0) {
    perror("fstat");
    fclose(file);
    return EXIT_FAILURE;
  }
  if (S_ISDIR(st.st_mode)) {
    fprintf(stderr, "error: %s is a directory\n", args.input);
    fclose(file);
    return EXIT_FAILURE;
  }

  setvbuf(stdout, NULL, _IOFBF, 65536);

  if (args.offset > 0) {
    if (fseek(file, 0, SEEK_END) == 0) {
      long file_size = ftell(file);
      if (file_size >= 0 && args.offset >= file_size) {
        fprintf(stderr, "error: offset (0x%lx) exceeds file size (%ld bytes)\n", args.offset,
                file_size);
        fclose(file);
        return EXIT_FAILURE;
      }
      rewind(file);
    } else if (errno == ESPIPE) {
      rewind(file);
    } else {
      perror("fseek");
      fclose(file);
      return EXIT_FAILURE;
    }
  }

  DumpStatistik stats = {0};
  if (!dump_file(file, &stats, args.offset, args.length, args.big_endian)) {
    fprintf(stderr, "error: failed to read file %s\n", args.input);
    fclose(file);
    return EXIT_FAILURE;
  }
  fclose(file);

  if (stats.total_lines == 0) {
    fprintf(stderr, "warning: file is empty, nothing to dump\n");
  }

  print_hasil(args.input, &stats);

  if (ferror(stdout)) {
    return EXIT_FAILURE;
  }
  return 0;
}
