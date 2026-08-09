#include "include/cli.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/color.h"
#include "include/pattern.h"

static const char* prog_name(const char* path) {
  const char* slash = strrchr(path, '/');
  if (slash) {
    return slash + 1;
  }

  return path;
}

void print_usage(const char* prog) {
  const char* name = prog_name(prog);
  fprintf(stderr,
          "usage: %s [options] <input> [output]\n"
          "\n"
          "Dump mode (default):\n"
          "  %s [options] <file>\n"
          "\n"
          "Reverse mode:\n"
          "  %s -r <dump.txt> <output.bin>\n"
          "\n"
          "Options:\n"
          "  -h, --help       Show this help\n"
          "  --version        Show version\n"
          "  -o <offset>      Start dump at offset (hex/dec)\n"
          "  -n <length>      Limit number of bytes to dump\n"
          "  -s <hex>         Search & highlight hex pattern\n"
          "  -S <string>      Search & highlight ASCII pattern\n"
          "  -b               Show big-endian 32-bit preview\n"
          "  -c               Disable colors\n"
          "  -r               Reverse hexdump to binary\n"
          "\n"
          "Examples:\n"
          "  %s /bin/ls\n"
          "  %s -c /bin/ls > dump.txt\n"
          "  %s -r dump.txt output.bin\n"
          "  %s -o 0x1000 -n 256 /bin/ls\n"
          "  %s -b /bin/ls\n",
          name, name, name, name, name, name, name, name, name);
}

bool parse_args(int argc, char* argv[], CliArgs* out) {
  out->input = NULL;
  out->output = NULL;
  out->reverse = false;
  out->offset = 0;
  out->length = 0;
  out->help = false;
  out->version = false;
  out->big_endian = false;

  int i = 1;
  while (i < argc) {
    if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
      if (!pattern_init_hex(argv[i + 1])) {
        fprintf(stderr, "error: invalid hex pattern\n");
        return false;
      }
      i += 2;
    } else if (strcmp(argv[i], "-S") == 0 && i + 1 < argc) {
      if (!pattern_init_ascii(argv[i + 1])) {
        fprintf(stderr, "error: invalid ASCII pattern\n");
        return false;
      }
      i += 2;
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      char* endptr;
      errno = 0;
      out->offset = strtol(argv[i + 1], &endptr, 0);
      if ((*endptr != '\0') || (errno == ERANGE)) {
        fprintf(stderr, "error: invalid offset: %s\n", argv[i + 1]);
        return false;
      }
      i += 2;
    } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
      char* endptr;
      errno = 0;
      out->length = strtoull(argv[i + 1], &endptr, 0);
      if ((*endptr != '\0') || (errno == ERANGE)) {
        fprintf(stderr, "error: invalid length: %s\n", argv[i + 1]);
        return false;
      }
      i += 2;
    } else if (strcmp(argv[i], "-b") == 0) {
      out->big_endian = true;
      i++;
    } else if (strcmp(argv[i], "-c") == 0) {
      color_disable();
      i++;
    } else if (strcmp(argv[i], "-r") == 0) {
      out->reverse = true;
      i++;
    } else if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0)) {
      out->help = true;
      return true;
    } else if (strcmp(argv[i], "--version") == 0) {
      out->version = true;
      return true;
    } else if (argv[i][0] == '-') {
      fprintf(stderr, "error: unknown option: %s\n", argv[i]);
      print_usage(argv[0]);
      return false;
    } else {
      if (out->input == NULL) {
        out->input = argv[i];
      } else if (out->output == NULL) {
        out->output = argv[i];
      } else {
        fprintf(stderr, "error: too many arguments\n");
        print_usage(argv[0]);
        return false;
      }
      i++;
    }
  }

  if (out->help || out->version) {
    return true;
  }

  if (out->input == NULL) {
    print_usage(argv[0]);
    return false;
  }
  if (out->reverse && out->output == NULL) {
    fprintf(stderr, "error: reverse mode requires an output file\n");
    print_usage(argv[0]);
    return false;
  }
  return true;
}
