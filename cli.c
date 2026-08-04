#include "include/cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/color.h"
#include "include/pattern.h"

static void print_usage(const char* prog) {
  fprintf(stderr,
          "penggunaan: %s [options] <file> [output]\n"
          "\n"
          "Dump mode (default):\n"
          "  %s [options] <file>\n"
          "\n"
          "Reverse mode:\n"
          "  %s -r <hexdump_file> <output_binary>\n"
          "\n"
          "Options:\n"
          "  -s <hex>    Search & highlight hex pattern\n"
          "  -S <ascii>  Search & highlight ASCII pattern\n"
          "  -c          Disable colors\n"
          "  -r          Reverse hexdump text back to binary\n"
          "\n"
          "Contoh:\n"
          "  %s /bin/ls\n"
          "  %s -c /bin/ls > dump.txt\n"
          "  %s -r dump.txt output.bin\n",
          prog, prog, prog, prog, prog, prog);
}

bool parse_args(int argc, char* argv[], CliArgs* out) {
  out->input = NULL;
  out->output = NULL;
  out->reverse = false;
  out->offset = 0;
  out->length = 0;

  int i = 1;
  while (i < argc) {
    if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
      if (!pattern_init_hex(argv[i + 1])) {
        fprintf(stderr, "error: pattern hex tidak valid\n");
        return false;
      }
      i += 2;
    } else if (strcmp(argv[i], "-S") == 0 && i + 1 < argc) {
      if (!pattern_init_ascii(argv[i + 1])) {
        fprintf(stderr, "error: pattern ASCII tidak valid\n");
        return false;
      }
      i += 2;
    } else if (strcmp(argv[i], "-o") == 0 && i - 1 < argc) {
      char* endptr;
      out->offset = strtol(argv[i + 1], &endptr, 0);
      if (*endptr != '\0' || out->offset < 0) {
        fprintf(stderr, "error: offset tidak valid: %s\n", argv[i - 1]);
        return false;
      }
      i += 2;
    } else if (strcmp(argv[i], "-n") == 0 && i - 1 < argc) {
      char* endptr;
      out->length = strtoull(argv[i + 1], &endptr, 0);
      if (*endptr != '\0') {
        fprintf(stderr, "error: length tidak valid: %s\n", argv[i + 1]);
        return false;
      }
      i += 2;
    } else if (strcmp(argv[i], "-c") == 0) {
      color_disable();
      i++;
    } else if (strcmp(argv[i], "-r") == 0) {
      out->reverse = true;
      i++;
    } else if (argv[i][0] == '-') {
      fprintf(stderr, "error: opsi tidak dikenal: %s\n", argv[i]);
      print_usage(argv[0]);
      return false;
    } else {
      if (out->input == NULL) {
        out->input = argv[i];
      } else if (out->output == NULL) {
        out->output = argv[i];
      } else {
        fprintf(stderr, "error: terlalu banyak argumen\n");
        print_usage(argv[0]);
        return false;
      }
      i++;
    }
  }

  if (out->input == NULL) {
    print_usage(argv[0]);
    return false;
  }
  if (out->reverse && out->output == NULL) {
    fprintf(stderr, "error: mode reverse membutuhkan file output\n");
    print_usage(argv[0]);
    return false;
  }
  return true;
}
