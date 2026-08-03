#include "include/cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/pattern.h"

static void print_usage(const char* prog) {
  fprintf(stderr,
          "penggunaan: %s [options] <file>\n"
          "\n"
          "Options:\n"
          "  -s <hex>    Search & highlight hex pattern (e.g., -s DEADBEEF)\n"
          "  -S <ascii>  Search & highlight ASCII pattern (e.g., -S \"flag\")\n"
          "\n"
          "Contoh:\n"
          "  %s /bin/ls\n"
          "  %s -s 7f454c46 /bin/ls        # cari magic ELF\n"
          "  %s -S \"flag{\" challenge.bin   # cari flag CTF\n",
          prog, prog, prog, prog);
}

const char* parse_args(int argc, char* argv[]) {
  const char* filename = NULL;
  int i = 1;

  while (i < argc) {
    if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
      if (!parse_hex_pattern(argv[i + 1])) {
        fprintf(stderr, "error: pattern hex tidak valid\n");
        return NULL;
      }
      g_has_pattern = true;
      i += 2;
    } else if (strcmp(argv[i], "-S") == 0 && i + 1 < argc) {
      if (!parse_ascii_pattern(argv[i + 1])) {
        fprintf(stderr, "error: pattern ASCII tidak valid\n");
        return NULL;
      }
      g_has_pattern = true;
      i += 2;
    } else if (argv[i][0] == '-') {
      fprintf(stderr, "error: opsi tidak dikenal: %s\n", argv[i]);
      print_usage(argv[0]);
      return NULL;
    } else {
      if (filename != NULL) {
        fprintf(stderr, "error: hanya boleh satu file\n");
        print_usage(argv[0]);
        return NULL;
      }
      filename = argv[i];
      i++;
    }
  }

  if (filename == NULL) {
    print_usage(argv[0]);
    return NULL;
  }
  return filename;
}
