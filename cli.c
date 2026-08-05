#include "include/cli.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/color.h"
#include "include/pattern.h"

static const char* prog_name(const char* path) {
  const char* slash = strrchr(path, '/');
  return slash ? slash + 1 : path;
}

void print_usage(const char* prog) {
  const char* name = prog_name(prog);
  fprintf(stderr,
          "penggunaan: %s [options] <file> [output]\n"
          "\n"
          "Dump mode (default):\n"
          "  %s [options] <file>\n"
          "\n"
          "Reverse mode:\n"
          "  %s -r <dump.txt> <output.bin>\n"
          "\n"
          "Options:\n"
          "  -h, --help          Tampilkan bantuan ini\n"
          "  --version           Tampilkan versi\n"
          "  -o <offset>         Mulai dump dari offset (hex/dec)\n"
          "  -n <length>         Batasi jumlah byte yang di-dump\n"
          "  -s <hex>            Cari & highlight pola hex\n"
          "  -S <ascii>          Cari & highlight pola ASCII\n"
          "  -c                  Matikan warna\n"
          "  -r                  Reverse hexdump ke biner\n"
          "\n"
          "Contoh:\n"
          "  %s /bin/ls\n"
          "  %s -c /bin/ls > dump.txt\n"
          "  %s -r dump.txt output.bin\n"
          "  %s -o 0x1000 -n 256 /bin/ls\n",
          name, name, name, name, name, name, name);
}

bool parse_args(int argc, char* argv[], CliArgs* out) {
  out->input = NULL;
  out->output = NULL;
  out->reverse = false;
  out->offset = 0;
  out->length = 0;
  out->help = false;
  out->version = false;

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
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      char* endptr;
      errno = 0;
      out->offset = strtol(argv[i + 1], &endptr, 0);
      if (*endptr != '\0' || out->offset < 0 || errno == ERANGE) {
        fprintf(stderr, "error: offset tidak valid: %s\n", argv[i + 1]);
        return false;
      }
      i += 2;
    } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
      char* endptr;
      errno = 0;
      out->length = strtoull(argv[i + 1], &endptr, 0);
      if (*endptr != '\0' || errno == ERANGE) {
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
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      out->help = true;
      return true;
    } else if (strcmp(argv[i], "--version") == 0) {
      out->version = true;
      return true;
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

  if (out->help || out->version) {
    return true;
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
