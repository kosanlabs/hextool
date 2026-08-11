#include "include/stats.h"

#include <math.h>
#include <stdio.h>

#include "include/color.h"
#include "include/config.h"
#include "include/elf.h"
#include "include/pattern.h"

static void print_frequency_summary(const DumpStatistik* stats);

static double safe_pct(size_t part, size_t total) {
  return total ? (part * 100.0 / total) : 0.0;
}

static void print_entropy_summary(const EntropyStats* s) {
  if (s->count == 0) {
    return;
  }

  double variance = s->m2 / (double)s->count;
  double stddev = sqrt(variance);

  printf("\n%sEntropy analysis (adaptive):%s\n", AC(CLR_BOLD), AC(CLR_RESET));
  printf("    Lines sampled          : %zu\n", s->count);
  printf("    Mean entropy           : %.3f bits/byte\n", s->mean);
  printf("    Std deviation          : %.3f\n", stddev);
  printf("    Range                  : %.3f .. %.3f\n", s->min, s->max);

  if (s->anomaly_high > 0 || s->anomaly_low > 0) {
    printf("    %sHigh anomalies (>+2 sigma) : %zu%s\n", AC(CLR_ENTROPY_HIGH), s->anomaly_high,
           AC(CLR_RESET));
    printf("    %sLow anomalies  (<-2 sigma) : %zu%s\n", AC(CLR_ENTROPY_LOW), s->anomaly_low,
           AC(CLR_RESET));
  } else {
    printf("    Anomalies (>2 sigma)        : none\n");
  }

  if (s->mean > 7.5) {
    printf("    %sHeuristic%s              : likely encrypted or compressed\n",
           AC(CLR_ENTROPY_HIGH), AC(CLR_RESET));
  } else if (s->mean < 2.0) {
    printf("    %sHeuristic%s              : likely sparse / padding\n", AC(CLR_ENTROPY_LOW),
           AC(CLR_RESET));
  } else if (stddev > 2.0) {
    printf("    %sHeuristic%s              : mixed content (code + data)\n", AC(CLR_ENTROPY),
           AC(CLR_RESET));
  }
}

static void print_segment_summary(const DumpStatistik* stats) {
  if (!stats->is_elf || stats->segment_count == 0) return;

  printf("\n%sSegment map:%s\n", AC(CLR_BOLD), AC(CLR_RESET));
  for (int i = 0; i < stats->segment_count; i++) {
    const ElfSegment* s = &stats->segments[i];
    const char* type_name = elf_segment_type_name(s->type);

    char flags[4] = {0};
    int f = 0;
    if (s->flags & 4) {
      flags[f++] = 'R';
    }
    if (s->flags & 2) {
      flags[f++] = 'W';
    }
    if (s->flags & 1) {
      flags[f++] = 'X';
    }

    printf("  %-14s 0x%08llx - 0x%08llx  (%8llu bytes)  %s\n", type_name,
           (unsigned long long)s->offset, (unsigned long long)(s->offset + s->filesz),
           (unsigned long long)s->filesz, flags);
  }
}

void print_hasil(const char* filename, const DumpStatistik* stats) {
  printf("\n\n");
  printf("File           : %s%s%s\n", AC(CLR_BOLD), filename, AC(CLR_RESET));
  printf("Total bytes    : %zu\n", stats->total_bytes);
  printf("Total lines    : %zu\n", stats->total_lines);

  if (stats->is_elf) {
    printf("Format         : %sELF Executable%s\n", AC(CLR_ELF), AC(CLR_RESET));
    printf("Architecture   : %s%s%s\n", AC(CLR_ARCH), machine_to_arch(stats->elf_machine),
           AC(CLR_RESET));
  }
  if (pattern_is_active()) {
    printf("Pattern        : %sSEARCH ACTIVE%s\n", AC(CLR_MATCH), AC(CLR_RESET));
  }

  if (stats->total_bytes == 0) {
    printf("\n  %s [no byte details for empty file]%s\n", AC(CLR_DIM), AC(CLR_RESET));
    return;
  }

  printf("\nByte details:\n");
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
  printf("    %s(adaptive after %d lines: z-score scale, ! = +2 sigma anomaly, . = -2 sigma)%s\n",
         AC(CLR_DIM), ENTROPY_WARMUP, AC(CLR_RESET));

  print_entropy_summary(&stats->entropy);

  print_frequency_summary(stats);

  print_segment_summary(stats);
}

static void print_frequency_summary(const DumpStatistik* stats) {
  if (stats->total_bytes == 0) {
    return;
  }

  typedef struct {
    unsigned char byte;
    size_t count;
  } FreqEntry;

  FreqEntry entries[256];
  int n = 0;

  for (int i = 0; i < 256; i++) {
    if (stats->freq[i] > 0) {
      entries[n].byte = (unsigned char)i;
      entries[n].count = stats->freq[i];
      n++;
    }
  }

  if (n == 0) {
    return;
  }

  for (int i = 1; i < n; i++) {
    FreqEntry key = entries[i];
    int j = i - 1;

    while (j >= 0 && entries[j].count < key.count) {
      entries[j + 1] = entries[j];
      j--;
    }
    entries[j + 1] = key;
  }

  int top = n < 10 ? n : 10;

  printf("\n%sFrequency analysis (top %d):%s\n", AC(CLR_BOLD), top, AC(CLR_RESET));

  for (int i = 0; i < top; i++) {
    double pct = safe_pct(entries[i].count, stats->total_bytes);
    int bar_len = (int)(pct / 100.0 * 20.0 + 0.5);

    if (bar_len < 1 && entries[i].count > 0) {
      bar_len = 1;
    }

    if (bar_len > 20) {
      bar_len = 20;
    }

    printf("  %s0x%02x%s : %6zu  (%5.1f%%)  ", AC(CLR_OFFSET), entries[i].byte, AC(CLR_RESET),
           entries[i].count, pct);

    for (int j = 0; j < bar_len; j++) {
      putchar('#');
    }

    putchar('\n');
  }
}
