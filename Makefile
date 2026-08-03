CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lm
TARGET = hextool

SRCS = main.c cli.c dump.c format.c color.c pattern.c elf.c stats.c
OBJS = $(SRCS:.c=.o)

.PHONY:: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
