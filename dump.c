#include "include/dump.h"

#include <ctype.h>
#include <stdio.h>

#include "include/color.h"
#include "include/config.h"
#include "include/elf.h"
#include "include/format.h"

bool dump_file(FILE* file, DumpStatistik* stats, long offset, size_t length) {
  unsigned char buffer[BYTES_PER_LINE];
  size_t bytes_read;
  size_t bytes_remaining = length;
  size_t total_dumped = 0;

  if (offset > 0) {
    if (fseek(file, offset, SEEK_SET) != 0) {
      perror("fseek");
      return false;
    }
  }

  size_t to_read = BYTES_PER_LINE;
  if (length > 0 && bytes_remaining < to_read) {
    to_read = bytes_remaining;
  }

  stats->is_elf = detect_elf(file);
  if (stats->is_elf) {
    stats->elf_machine = read_elf_machine(file);
  }

  while ((bytes_read = fread(buffer, 1, BYTES_PER_LINE, file)) > 0) {
    print_offset(offset);
    print_entropy_bar(buffer, bytes_read);
    print_le32(buffer, bytes_read);
    print_hex(buffer, bytes_read, stats->is_elf, offset);
    putchar(' ');
    print_ascii(buffer, bytes_read, stats->is_elf, offset);
    print_inline_strings(buffer, bytes_read);

    total_dumped += bytes_read;
    if (length > 0) {
      if (total_dumped >= length) {
        break;
      }

      bytes_remaining = length - total_dumped;
      to_read = (bytes_remaining < BYTES_PER_LINE) ? bytes_remaining : BYTES_PER_LINE;
    }

    if (stats->is_elf && offset < ELF_HEADER_SIZE) {
      const char* field = elf_field_name(offset);
      if (field) {
        printf(" %s; %s%s", AC(CLR_COMMENT), field, AC(CLR_RESET));
      }
    }
    putchar('\n');

    for (size_t i = 0; i < bytes_read; i++) {
      unsigned char c = buffer[i];
      if (c == 0x00) {
        stats->null_count++;
      } else if (c >= 0x80) {
        stats->high_count++;
      } else if (isprint(c)) {
        stats->print_count++;
      }
    }

    offset += bytes_read;
    stats->total_bytes += bytes_read;
    stats->total_lines++;
  }

  return !ferror(file);
}
