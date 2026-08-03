#ifndef CONFIG_H
#define CONFIG_H

enum { 
  BYTES_PER_LINE = 16,
  ELF_HEADER_SIZE = 64,
  ENTROPY_BAR_WIDTH = 8,
  MIN_STRING_LEN = 4,
  MAX_PATTERN_LEN = 256,
};

enum {
  ELF_MAGIC_SIZE = 4,
  ELF_MACHINE_OFFSET = 18,
  ELF_MACHINE_SIZE = 2
};

#define CLR_RESET "\x1b[0m"
#define CLR_OFFSET "\x1b[35m"
#define CLR_NULL "\x1b[38;5;240m"
#define CLR_PRINT "\x1b[32m"
#define CLR_LOW "\x1b[31m"
#define CLR_HIGH "\x1b[33m"
#define CLR_ELF "\x1b[35;1m"
#define CLR_ASCII "\x1b[90m"
#define CLR_LE "\x1b[34m"
#define CLR_MATCH "\x1b[48;5;196m\x1b[37;1m"
#define CLR_STR "\x1b[38;5;124m"
#define CLR_ENTROPY "\x1b[38;5;39m"
#define CLR_ARCH "\x1b[36;1m"
#define CLR_BOLD "\x1b[1m"
#define CLR_DIM "\x1b[2m"
#define CLR_COMMENT "\1xb[90m"

#endif // !CONFIG_H
