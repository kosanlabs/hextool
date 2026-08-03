#include "include/stats.h"

#include <stdio.h>

#include "include/config.h"
#include "include/elf.h"
#include "include/pattern.h"

static double safe_pct(size_t part, size_t total) {
  return total ? (part * 100.0 / total) : 0.0;
}

void print_hasil(const char* filename, const DumpStatistik* stats) {
  printf("\n\n");
  printf("File           : " CLR_BOLD "%s" CLR_RESET "\n", filename);
  printf("total bytesnya : %zu\n", stats->total_bytes);
  printf("total line     : %zu\n", stats->total_lines);

  if (stats->is_elf) {
    printf("Format         : " CLR_ELF "ELF Executable" CLR_RESET "\n");
    printf("Arsitektur CPU : " CLR_ARCH "%s" CLR_RESET "\n", machine_to_arch(stats->elf_machine));
  }
  if (pattern_is_active()) {
    printf("Pattern        : " CLR_MATCH "SEARCH AKTIF" CLR_RESET "\n");
  }

  printf("\nDetail byte:\n");
  double pct_null = safe_pct(stats->null_count, stats->total_bytes);
  double pct_print = safe_pct(stats->print_count, stats->total_bytes);
  double pct_high = safe_pct(stats->high_count, stats->total_bytes);
  size_t ctrl_count =
      stats->total_bytes - stats->null_count - stats->print_count - stats->high_count;
  double pct_ctrl = safe_pct(ctrl_count, stats->total_bytes);

  printf("    " CLR_NULL "#" CLR_RESET " Null bytes (0x00)      : %zu (%.1f%%)\n",
         stats->null_count, pct_null);
  printf("    " CLR_PRINT "#" CLR_RESET " Printable ASCII        : %zu (%.1f%%)\n",
         stats->print_count, pct_print);
  printf("    " CLR_HIGH "#" CLR_RESET " High bytes (>= 0x80)   : %zu (%.1f%%)\n",
         stats->high_count, pct_high);
  printf("    " CLR_LOW "#" CLR_RESET " Control / Low bytes    : %zu (%.1f%%)\n", ctrl_count,
         pct_ctrl);

  printf("\n" CLR_DIM "Entropy bar: [| = high randomness, empty = low randomness]" CLR_RESET "\n");
}
