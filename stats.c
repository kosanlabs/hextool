#include "stats.h"

#include <stdio.h>

#include "config.h"
#include "pattern.h"

void print_hasil(const char* filename, const DumpStatistik* stats) {
  printf("\n\n");
  printf("File           : " CLR_BOLD "%s" CLR_RESET "\n", filename);

  printf("total bytesnya : %zu\n", stats->total_bytes);
  printf("total line     : %zu\n", stats->total_lines);

  if (stats->is_elf) {
    printf("Format         :" CLR_ELF " Elf Executable " CLR_RESET "\n");
  }

  if (g_has_pattern) {
    printf("Pattern        :" CLR_MATCH " SEARCH AKTIF " CLR_RESET "\n");
  }

  printf("\n");
  printf("Detail byte:\n");
  printf("    " CLR_NULL "#" CLR_RESET " Null bytes (0x00) : %zu (%.1f%%)\n", stats->null_count,
         stats->total_bytes ? (stats->null_count * 100.0 / stats->total_bytes) : 0);
  printf("    " CLR_PRINT "#" CLR_RESET " Printable ascii : %zu (%.1f%%)\n", stats->print_count,
         stats->total_bytes ? (stats->print_count * 100.0 / stats->total_bytes) : 0);
  printf("    " CLR_HIGH "#" CLR_RESET " High bytes (>= 0x80): %zu (%.1f%%)\n", stats->high_count,
         stats->total_bytes ? (stats->high_count * 100.0 / stats->total_bytes) : 0);
  printf("    " CLR_LOW "#" CLR_RESET " Control | Low bytes (>= 0x80): %zu (%.1f%%)\n",
         stats->total_bytes - stats->null_count - stats->print_count - stats->high_count,
         stats->total_bytes
             ? ((stats->total_bytes - stats->null_count - stats->print_count - stats->high_count) *
                100.0 / stats->total_bytes)
             : 0);
  printf("\n" CLR_DIM "Entropy bar: [| = high randomness, empty = low randomness]" CLR_RESET "\n");
}
