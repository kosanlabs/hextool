#include "include/stats.h"

#include <stdio.h>

#include "include/color.h"
#include "include/config.h"
#include "include/elf.h"
#include "include/pattern.h"

static double safe_pct(size_t part, size_t total) {
  return total ? (part * 100.0 / total) : 0.0;
}

void print_hasil(const char* filename, const DumpStatistik* stats) {
  printf("\n\n");
  printf("File           : %s%s%s\n", AC(CLR_BOLD), filename, AC(CLR_RESET));
  printf("total bytesnya : %zu\n", stats->total_bytes);
  printf("total line     : %zu\n", stats->total_lines);

  if (stats->is_elf) {
    printf("Format         : %sELF Executable%s\n", AC(CLR_ELF), AC(CLR_RESET));
    printf("Arsitektur CPU : %s%s%s\n", AC(CLR_ARCH), machine_to_arch(stats->elf_machine),
           AC(CLR_RESET));
  }
  if (pattern_is_active()) {
    printf("Pattern        : %sSEARCH AKTIF%s\n", AC(CLR_MATCH), AC(CLR_RESET));
  }

  if (stats->total_bytes == 0) {
    printf("\n %s [tidak ada detail byte untuk file kosong]%s\n", AC(CLR_DIM), AC(CLR_RESET));
    return;
  }

  printf("\nDetail byte:\n");
  double pct_null = safe_pct(stats->null_count, stats->total_bytes);
  double pct_print = safe_pct(stats->print_count, stats->total_bytes);
  double pct_high = safe_pct(stats->high_count, stats->total_bytes);
  size_t ctrl_count =
      stats->total_bytes - stats->null_count - stats->print_count - stats->high_count;
  double pct_ctrl = safe_pct(ctrl_count, stats->total_bytes);

  printf("    %s#%s Null bytes (0x00)      : %zu (%.1f%%)\n", AC(CLR_NULL), AC(CLR_RESET),
         stats->null_count, pct_null);
  printf("    %s#%s Printable ASCII        : %zu (%.1f%%)\n", AC(CLR_PRINT), AC(CLR_RESET),
         stats->print_count, pct_print);
  printf("    %s#%s High bytes (>= 0x80)   : %zu (%.1f%%)\n", AC(CLR_HIGH), AC(CLR_RESET),
         stats->high_count, pct_high);
  printf("    %s#%s Control / Low bytes    : %zu (%.1f%%)\n", AC(CLR_LOW), AC(CLR_RESET),
         ctrl_count, pct_ctrl);

  printf("\n%sEntropy bar: [| = high randomness, empty = low randomness]%s\n", AC(CLR_DIM),
         AC(CLR_RESET));
}
