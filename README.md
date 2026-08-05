# Warga slowy HexDump

A basic hexdump implementation in C.

## Goals

- binary file i/o
- hex dump
- read elf header
- read cpu architecture
- binary parsing
- read little endian
- reverse hexdump

## Usage

```bash
# build with make
make

# remove executables and object files
make clean

# example: ./hextool file_binary
./hextool file_binary

# search & highlight hex pattern (e.g. ELF magic)
./hextool -s 7f454c46 file_binary

# search & highlight ascii pattern (e.g. CTF flag)
./hextool -S "flag{" file_binary

# disable colors (useful when redirecting output to a file)
./hextool -c file_binary > dump.txt

# reverse hexdump to binary
./hextool -r dump.txt output.bin

# dump 256 bytes from offset 0x1000
./hextool -o 0x1000 -n 256 file_binary

# dump only the ELF header (first 64 bytes)
./hextool -n 64 file_binary

# dump a specific offset and search for a string
./hextool -o 0x2f20 -n 128 -S "libc" file_binary
```

## Output result

```
....
00006660  [|       ] |00000011| 11 00 00 00 03 00 00 00  00 00 00 00 00 00 00 00  |................|
00006670  [|       ] |00000000| 00 00 00 00 00 00 00 00  C2 5D 00 00 00 00 00 00  |.........]......|
00006680  [|       ] |0000011e| 1E 01 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00006690  [        ] |00000001| 01 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|


File           : hextool
Total bytes    : 26272
Total lines    : 1642
Format         : ELF Executable
Architecture   : x86-64

Byte details:
    # Null bytes (0x00)      : 15400 (58.6%)
    # Printable ASCII        : 5568 (21.2%)
    # High bytes (>= 0x80)   : 3154 (12.0%)
    # Control / Low bytes    : 2150 (8.2%)

Entropy bar: [| = high randomness, empty = low randomness]
```
