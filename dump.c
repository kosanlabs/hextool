#define _POSIX_C_SOURCE 200809L

#include "include/dump.h"

#include <ctype.h>
#include <stdio.h>

#include "include/color.h"
#include "include/config.h"
#include "include/elf.h"
#include "include/format.h"

bool dump_file(FILE* file, DumpStatistik* stats, long start_offset, size_t max_length) {
  unsigned char buffer[BYTES_PER_LINE];
  size_t offset = (size_t)start_offset;
  size_t bytes_read;
  size_t dumped = 0;

  if (start_offset == 0) {
    stats->is_elf = detect_elf(file);
    if (stats->is_elf) {
      stats->elf_machine = read_elf_machine(file);
      rewind(file);
    }
  } else {
    stats->is_elf = false;
  }

#ifdef __linux__
#include <fcntl.h>
  posix_fadvise(fileno(file), start_offset, max_length ? (off_t)max_length : 0,
                POSIX_FADV_SEQUENTIAL);
#endif /* ifdef __linux__ */

  if (offset > 0) {
    if (fseek(file, offset, SEEK_SET) != 0) {
      perror("fseek");
      return false;
    }
  }

  while (1) {
    size_t to_read = BYTES_PER_LINE;
    if (max_length > 0) {
      size_t remain = max_length - dumped;
      if (remain == 0) {
        break;
      }
      if (remain < to_read) {
        to_read = remain;
      }
    }

    bytes_read = fread(buffer, 1, to_read, file);
    if (bytes_read == 0) {
      break;
    }

    print_offset(offset);
    print_entropy_bar(buffer, bytes_read);
    print_le32(buffer, bytes_read);
    print_hex(buffer, bytes_read, stats->is_elf, offset);
    putchar(' ');
    print_ascii(buffer, bytes_read, stats->is_elf, offset);
    print_inline_strings(buffer, bytes_read);

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
    dumped += bytes_read;
    stats->total_bytes += bytes_read;
    stats->total_lines++;
  }

  return !ferror(file);
}
