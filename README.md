# Warga slowy HexDump

implement dasar dari hexdump dengan C.

## Tujuannya

- binary file i/o
- hex dump
- baca elf header
- baca arsitektur cpu
- ngeparsing biner
- baca little endian

## Cara Menjalankannya

```bash
# pakai library math.h untuk fungsi log2
# tambahkan juga parameter -lm
gcc -o contoh contoh.c -lm

# contoh: ./contoh contoh
./contoh file_biner
```

## Output result

```
....
000041e0      |00000001|  1F45902B 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013  1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013   |................|
000041f0      |00000011|  1F45902B 1F459013 1F459013 1F459013 1F45902B 1F459013 1F459013 1F459013  1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013   |................|
00004200      |00000000|  1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013  1F459025 1F459025 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013   |........P9......|
00004210      |0000011e|  1F45902B 1F45902B 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013  1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013   |................|
00004220      |00000001|  1F45902B 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013  1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013 1F459013   |................|


File           : contoh
total bytesnya : 16944
total line     : 1059
Format         : Elf Executable 

Detail byte:
    # Null bytes (0x00) : 10145 (59.9%)
    # Printable ascii : 3450 (20.4%)
    # High bytes (>= 0x80): 1976 (11.7%)
    # Control / Low bytes (>= 0x80): 1373 (8.1%)
```
