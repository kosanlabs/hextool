CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c17 -fsanitize=address -D_POSIX_C_SOURCE=200809L -D_FILE_OFFSET_BITS=64 -Iinclude
LDFLAGS = -lm
TARGET = hextool

SRCS = main.c cli.c dump.c format.c color.c pattern.c elf.c stats.c reverse.c diff.c binfmt.c disasm.c
OBJS = $(SRCS:.c=.o)

TEST_SRCS = tests/test_cli.c tests/test_pattern.c tests/test_elf.c tests/test_reverse.c \
            tests/test_format.c tests/test_diff.c tests/test_dump.c tests/test_binfmt.c tests/test_disasm.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_BINS = $(TEST_SRCS:.c=)

.PHONY:: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(TEST_BINS)
	@f=0; for t in $(TEST_BINS); do \
	  echo "== $$t =="; \
	  ./$$t || f=1; \
	done; exit $$f

tests/test_cli: tests/test_cli.o cli.o pattern.o color.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
tests/test_pattern: tests/test_pattern.o pattern.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
tests/test_elf: tests/test_elf.o elf.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
tests/test_reverse: tests/test_reverse.o reverse.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
tests/test_format: tests/test_format.o format.o color.o pattern.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
tests/test_diff: tests/test_diff.o diff.o format.o color.o pattern.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
tests/test_dump: tests/test_dump.o dump.o format.o color.o pattern.o elf.o binfmt.o disasm.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
tests/test_binfmt: tests/test_binfmt.o binfmt.o elf.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
tests/test_disasm: tests/test_disasm.o disasm.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_BINS) tests/test.o
