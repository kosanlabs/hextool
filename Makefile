CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c17 -D_POSIX_C_SOURCE=200809L -D_FILE_OFFSET_BITS=64 -Iinclude
LDFLAGS = -lm
TARGET = hextool
TEST_TARGET = test_runner

SRCS = main.c cli.c dump.c format.c color.c pattern.c elf.c stats.c reverse.c
OBJS = $(SRCS:.c=.o)

TEST_SRCS = tests/test.c cli.c pattern.c elf.c reverse.c color.c format.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

.PHONY:: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test:	$(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_TARGET)
