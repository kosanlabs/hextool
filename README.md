# Warga slowy HexDump

implement dasar dari hexdump dengan C.

## Tujuannya

- binary file i/o
- hex dump
- baca elf header
- baca arsitektur cpu
- ngeparsing biner
- baca little endian
- reverse hexdump

## Cara Menjalankannya

```bash
# pakai makefile tinggal make
make

# untuk clear file executable dan object file
make clean

# contoh: ./hextool file_biner
./hextool file_biner

# cari & highlight pola hex (misal magic ELF)
./hextool -s 7f454c46 file_biner

# cari & highlight pola ascii (misal flag CTF)
./hextool -S "flag{" file_biner

# matikan warna (berguna kalau output diarahkan ke file)
./hextool -c file_biner > dump.txt

# reverse hexdump jadi biner
./hextool -r dump.txt output.bin
```

## Output result

```
....
00006660  [|       ] |00000011| 11 00 00 00 03 00 00 00  00 00 00 00 00 00 00 00  |................|
00006670  [|       ] |00000000| 00 00 00 00 00 00 00 00  C2 5D 00 00 00 00 00 00  |.........]......|
00006680  [|       ] |0000011e| 1E 01 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00006690  [        ] |00000001| 01 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|


File           : hextool
total bytesnya : 26272
total line     : 1642
Format         : ELF Executable
Arsitektur CPU : x86-64

Detail byte:
    # Null bytes (0x00)      : 15400 (58.6%)
    # Printable ASCII        : 5568 (21.2%)
    # High bytes (>= 0x80)   : 3154 (12.0%)
    # Control / Low bytes    : 2150 (8.2%)

Entropy bar: [| = high randomness, empty = low randomness]
```
