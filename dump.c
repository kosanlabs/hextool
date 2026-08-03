#include "dump.h"
#include "config.h"
#include "format.h"
#include "elf.h"
#include <stdio.h>
#include <ctype.h>

void dump_file(FILE* file, DumpStatistik* stats) {
    unsigned char buffer[BYTES_PER_LINE];
    size_t offset = 0;
    size_t bytes_read;

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

        if (stats->is_elf && offset < ELF_HEADER_SIZE) {
            const char* field = elf_field_name(offset);
            if (field) printf(" \x1b[90m; %s\x1b[0m", field);
        }
        putchar('\n');

        for (size_t i = 0; i < bytes_read; i++) {
            unsigned char c = buffer[i];
            if (c == 0x00)      stats->null_count++;
            else if (c >= 0x80) stats->high_count++;
            else if (isprint(c)) stats->print_count++;
        }

        offset += bytes_read;
        stats->total_bytes += bytes_read;
        stats->total_lines++;
    }
}
