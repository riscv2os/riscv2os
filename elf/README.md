# ELF 目的檔格式

參考 

* [Linux ELF 二進位檔案入門：搞懂兼分析](https://security-onigiri.github.io/2018/03/08/the-101-of-elf-binaries-on-linux-understanding-and-analysis.html)
* http://labmaster.mi.infn.it/Laboratorio2/CompilerCD/clang/l1/ELF.html

## run

```
$ ./dumpelf oshello.elf
ELF:
  type:2        (1=ELF32, 2=ELF64)
  machine:243   (40=ARM, 50=IA_64, 243=RISCV)
  version:1
  flags:0
  entry:0x80000000

Section Headers:(type: 1=PROGBITS, 2=SYMTAB, 3=STRTAB)
  [Nr] Name                 Type            Address          Off    Size   ES Flg Lk Inf Al
                            0               0000000000000000 000000 000000 00   0  0   0  0
       .text                1               0000000080000000 001000 0080e4 00   6  0   0  4
       .rodata              1               00000000800080e4 0090e4 00000b 00   2  0   0  4
       .data                1               00000000800080ef 0090ef 000f11 00   3  0   0  1
       .riscv.attributes    70000003        0000000000000000 00a000 000026 00   0  0   0  1
       .comment             1               0000000000000000 00a026 000028 01  30  0   0  1
       .symtab              2               0000000000000000 00a050 0000f0 10   0  7  11  4
       .strtab              3               0000000000000000 00a140 00006a 00   0  0   0  1
       .shstrtab            3               0000000000000000 00a1aa 00004a 00   0  0   0  1

Program Headers: (type: 1=LOAD, 2=DYNAMIC, ... SHLIB=5 ...)
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  1              0x001000 0x0000000080000000 0x0000000080000000 0x0080ef 0x0080ef   5 0x1000
  1              0x0090ef 0x00000000800080ef 0x00000000800080ef 0x000f11 0x000f11   6 0x1000
  1              0x00a000 0x0000000000000000 0x0000000000000000 0x000000 0x000000   0 0x1000

string table:
 .symtab .strtab .shstrtab .text .rodata .data .riscv.attributes .comment 



.text
f3 22 40 f1 93 92 a2 00 17 21 00 00 13 01 01 02 33 01 51 00 73 25 40 f1 63 14 05 00 6f 80 80 0a 73 00 50 10 6f f0 df ff 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ...

.rodata
48 65 6c 6c 6f 20 4f 53 21 0a 00

.data
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ...

.riscv.attributes
41 25 00 00 00 72 69 73 63 76 00 01 1b 00 00 00 04 10 05 72 76 33 32 69 32 70 30 5f 6d 32 70 30 5f 61 32 70 30 00 

.comment
47 43 43 3a 20 28 53 69 46 69 76 65 20 47 43 43 20 38 2e 33 2e 30 2d 32 30 32 30 2e 30 34 2e 31 29 20 38 2e 33 2e 30 00 

.symtab
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 00 00 00 00 03 00 01 00 00 00 00 00 e4 80 00 80 00 00 
00 00 03 00 02 00 00 00 00 00 ef 80 00 80 00 00 00 00 03 00 03 00 00 00 00 00 00 00 00 00 00 00 00 00 03 00 04 00 00 00 00 00 
00 00 00 00 00 00 00 00 03 00 05 00 01 00 00 00 00 00 00 00 00 00 00 00 04 00 f1 ff 2d 00 00 00 00 20 00 00 00 00 00 00 00 00 
f1 ff 38 00 00 00 28 00 00 80 00 00 00 00 00 00 01 00 3f 00 00 00 20 00 00 80 00 00 00 00 00 00 01 00 44 00 00 00 00 00 00 00 
00 00 00 00 04 00 f1 ff 49 00 00 00 28 80 00 80 4c 00 00 00 12 00 01 00 52 00 00 00 74 80 00 80 50 ...

.strtab
00 43 3a 5c 55 73 65 72 73 5c 75 73 65 72 5c 41 70 70 44 61 74 61 5c 4c 6f 63 61 6c 5c 54 65 6d 70 5c 63 63 4e 70 77 38 4f 62 
2e 6f 00 53 54 41 43 4b 5f 53 49 5a 45 00 73 74 61 63 6b 73 00 70 61 72 6b 00 6f 73 2e 63 00 6c 69 62 5f 70 75 74 63 00 6c 69 
62 5f 70 75 74 73 00 6f 73 5f 6d 61 69 6e 00 5f 73 74 61 72 74 00

.shstrtab
00 2e 73 79 6d 74 61 62 00 2e 73 74 72 74 61 62 00 2e 73 68 73 74 72 74 61 62 00 2e 74 65 78 74 00 2e 72 6f 64 61 74 61 00 2e 
64 61 74 61 00 2e 72 69 73 63 76 2e 61 74 74 72 69 62 75 74 65 73 00 2e 63 6f 6d 6d 65 6e 74 00 

```


## tool

```
readelf --wide elf3 --all > elf3.txt
```

結果

```
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              DYN (Shared object file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x750
  Start of program headers:          64 (bytes into file)
  Start of section headers:          10992 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         9
  Size of section headers:           64 (bytes)
  Number of section headers:         29
  Section header string table index: 28

Section Headers:
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
  [ 1] .interp           PROGBITS        0000000000000238 000238 00001c 00   A  0   0  1
  [ 2] .note.ABI-tag     NOTE            0000000000000254 000254 000020 00   A  0   0  4
  [ 3] .note.gnu.build-id NOTE            0000000000000274 000274 000024 00   A  0   0  4
  [ 4] .gnu.hash         GNU_HASH        0000000000000298 000298 00001c 00   A  5   0  8
  [ 5] .dynsym           DYNSYM          00000000000002b8 0002b8 000150 18   A  6   1  8
  [ 6] .dynstr           STRTAB          0000000000000408 000408 0000c4 00   A  0   0  1
  [ 7] .gnu.version      VERSYM          00000000000004cc 0004cc 00001c 02   A  5   0  2
  [ 8] .gnu.version_r    VERNEED         00000000000004e8 0004e8 000030 00   A  6   1  8
  [ 9] .rela.dyn         RELA            0000000000000518 000518 0000c0 18   A  5   0  8
  [10] .rela.plt         RELA            00000000000005d8 0005d8 0000c0 18  AI  5  22  8
  [11] .init             PROGBITS        0000000000000698 000698 000017 00  AX  0   0  4
  [12] .plt              PROGBITS        00000000000006b0 0006b0 000090 10  AX  0   0 16
  [13] .plt.got          PROGBITS        0000000000000740 000740 000008 08  AX  0   0  8
  [14] .text             PROGBITS        0000000000000750 000750 000672 00  AX  0   0 16
  [15] .fini             PROGBITS        0000000000000dc4 000dc4 000009 00  AX  0   0  4
  [16] .rodata           PROGBITS        0000000000000dd0 000dd0 00022a 00   A  0   0  8
  [17] .eh_frame_hdr     PROGBITS        0000000000000ffc 000ffc 00005c 00   A  0   0  4
  [18] .eh_frame         PROGBITS        0000000000001058 001058 000188 00   A  0   0  8
  [19] .init_array       INIT_ARRAY      0000000000201d80 001d80 000008 08  WA  0   0  8
  [20] .fini_array       FINI_ARRAY      0000000000201d88 001d88 000008 08  WA  0   0  8
  [21] .dynamic          DYNAMIC         0000000000201d90 001d90 0001f0 10  WA  6   0  8
  [22] .got              PROGBITS        0000000000201f80 001f80 000080 08  WA  0   0  8
  [23] .data             PROGBITS        0000000000202000 002000 000010 00  WA  0   0  8
  [24] .bss              NOBITS          0000000000202010 002010 000008 00  WA  0   0  1
  [25] .comment          PROGBITS        0000000000000000 002010 00002b 01  MS  0   0  1
  [26] .symtab           SYMTAB          0000000000000000 002040 0006f0 18     27  43  8
  [27] .strtab           STRTAB          0000000000000000 002730 0002bd 00      0   0  1
  [28] .shstrtab         STRTAB          0000000000000000 0029ed 0000fe 00      0   0  1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  l (large), p (processor specific)

There are no section groups in this file.

Program Headers:
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  PHDR           0x000040 0x0000000000000040 0x0000000000000040 0x0001f8 0x0001f8 R   0x8
  INTERP         0x000238 0x0000000000000238 0x0000000000000238 0x00001c 0x00001c R   0x1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x000000 0x0000000000000000 0x0000000000000000 0x0011e0 0x0011e0 R E 0x200000
  LOAD           0x001d80 0x0000000000201d80 0x0000000000201d80 0x000290 0x000298 RW  0x200000
  DYNAMIC        0x001d90 0x0000000000201d90 0x0000000000201d90 0x0001f0 0x0001f0 RW  0x8
  NOTE           0x000254 0x0000000000000254 0x0000000000000254 0x000044 0x000044 R   0x4
  GNU_EH_FRAME   0x000ffc 0x0000000000000ffc 0x0000000000000ffc 0x00005c 0x00005c R   0x4
  GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x10
  GNU_RELRO      0x001d80 0x0000000000201d80 0x0000000000201d80 0x000280 0x000280 R   0x1

 Section to Segment mapping:
  Segment Sections...
   00     
   01     .interp 
   02     .interp .note.ABI-tag .note.gnu.build-id .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt .init .plt .plt.got .text .fini .rodata .eh_frame_hdr .eh_frame 
   03     .init_array .fini_array .dynamic .got .data .bss 
   04     .dynamic 
   05     .note.ABI-tag .note.gnu.build-id 
   06     .eh_frame_hdr 
   07     
   08     .init_array .fini_array .dynamic .got 

Dynamic section at offset 0x1d90 contains 27 entries:
  Tag        Type                         Name/Value
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
 0x000000000000000c (INIT)               0x698
 0x000000000000000d (FINI)               0xdc4
 0x0000000000000019 (INIT_ARRAY)         0x201d80
 0x000000000000001b (INIT_ARRAYSZ)       8 (bytes)
 0x000000000000001a (FINI_ARRAY)         0x201d88
 0x000000000000001c (FINI_ARRAYSZ)       8 (bytes)
 0x000000006ffffef5 (GNU_HASH)           0x298
 0x0000000000000005 (STRTAB)             0x408
 0x0000000000000006 (SYMTAB)             0x2b8
 0x000000000000000a (STRSZ)              196 (bytes)
 0x000000000000000b (SYMENT)             24 (bytes)
 0x0000000000000015 (DEBUG)              0x0
 0x0000000000000003 (PLTGOT)             0x201f80
 0x0000000000000002 (PLTRELSZ)           192 (bytes)
 0x0000000000000014 (PLTREL)             RELA
 0x0000000000000017 (JMPREL)             0x5d8
 0x0000000000000007 (RELA)               0x518
 0x0000000000000008 (RELASZ)             192 (bytes)
 0x0000000000000009 (RELAENT)            24 (bytes)
 0x000000000000001e (FLAGS)              BIND_NOW
 0x000000006ffffffb (FLAGS_1)            Flags: NOW PIE
 0x000000006ffffffe (VERNEED)            0x4e8
 0x000000006fffffff (VERNEEDNUM)         1
 0x000000006ffffff0 (VERSYM)             0x4cc
 0x000000006ffffff9 (RELACOUNT)          3
 0x0000000000000000 (NULL)               0x0

Relocation section '.rela.dyn' at offset 0x518 contains 8 entries:
    Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
0000000000201d80  0000000000000008 R_X86_64_RELATIVE                         850
0000000000201d88  0000000000000008 R_X86_64_RELATIVE                         810
0000000000202008  0000000000000008 R_X86_64_RELATIVE                         202008
0000000000201fd8  0000000100000006 R_X86_64_GLOB_DAT      0000000000000000 _ITM_deregisterTMCloneTable + 0
0000000000201fe0  0000000700000006 R_X86_64_GLOB_DAT      0000000000000000 __libc_start_main@GLIBC_2.2.5 + 0
0000000000201fe8  0000000900000006 R_X86_64_GLOB_DAT      0000000000000000 __gmon_start__ + 0
0000000000201ff0  0000000c00000006 R_X86_64_GLOB_DAT      0000000000000000 _ITM_registerTMCloneTable + 0
0000000000201ff8  0000000d00000006 R_X86_64_GLOB_DAT      0000000000000000 __cxa_finalize@GLIBC_2.2.5 + 0

Relocation section '.rela.plt' at offset 0x5d8 contains 8 entries:
    Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
0000000000201f98  0000000200000007 R_X86_64_JUMP_SLOT     0000000000000000 puts@GLIBC_2.2.5 + 0
0000000000201fa0  0000000300000007 R_X86_64_JUMP_SLOT     0000000000000000 fread@GLIBC_2.2.5 + 0
0000000000201fa8  0000000400000007 R_X86_64_JUMP_SLOT     0000000000000000 fclose@GLIBC_2.2.5 + 0
0000000000201fb0  0000000500000007 R_X86_64_JUMP_SLOT     0000000000000000 __stack_chk_fail@GLIBC_2.4 + 0
0000000000201fb8  0000000600000007 R_X86_64_JUMP_SLOT     0000000000000000 printf@GLIBC_2.2.5 + 0
0000000000201fc0  0000000800000007 R_X86_64_JUMP_SLOT     0000000000000000 memcmp@GLIBC_2.2.5 + 0
0000000000201fc8  0000000a00000007 R_X86_64_JUMP_SLOT     0000000000000000 fseek@GLIBC_2.2.5 + 0
0000000000201fd0  0000000b00000007 R_X86_64_JUMP_SLOT     0000000000000000 fopen@GLIBC_2.2.5 + 0

The decoding of unwind sections for machine type Advanced Micro Devices X86-64 is not currently supported.

Symbol table '.dynsym' contains 14 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTable
     2: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@GLIBC_2.2.5 (2)
     3: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fread@GLIBC_2.2.5 (2)
     4: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fclose@GLIBC_2.2.5 (2)
     5: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __stack_chk_fail@GLIBC_2.4 (3)
     6: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@GLIBC_2.2.5 (2)
     7: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.2.5 (2)
     8: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND memcmp@GLIBC_2.2.5 (2)
     9: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
    10: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fseek@GLIBC_2.2.5 (2)
    11: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fopen@GLIBC_2.2.5 (2)
    12: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
    13: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND __cxa_finalize@GLIBC_2.2.5 (2)

Symbol table '.symtab' contains 74 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000238     0 SECTION LOCAL  DEFAULT    1 
     2: 0000000000000254     0 SECTION LOCAL  DEFAULT    2 
     3: 0000000000000274     0 SECTION LOCAL  DEFAULT    3 
     4: 0000000000000298     0 SECTION LOCAL  DEFAULT    4 
     5: 00000000000002b8     0 SECTION LOCAL  DEFAULT    5 
     6: 0000000000000408     0 SECTION LOCAL  DEFAULT    6 
     7: 00000000000004cc     0 SECTION LOCAL  DEFAULT    7 
     8: 00000000000004e8     0 SECTION LOCAL  DEFAULT    8 
     9: 0000000000000518     0 SECTION LOCAL  DEFAULT    9 
    10: 00000000000005d8     0 SECTION LOCAL  DEFAULT   10 
    11: 0000000000000698     0 SECTION LOCAL  DEFAULT   11 
    12: 00000000000006b0     0 SECTION LOCAL  DEFAULT   12 
    13: 0000000000000740     0 SECTION LOCAL  DEFAULT   13 
    14: 0000000000000750     0 SECTION LOCAL  DEFAULT   14 
    15: 0000000000000dc4     0 SECTION LOCAL  DEFAULT   15 
    16: 0000000000000dd0     0 SECTION LOCAL  DEFAULT   16 
    17: 0000000000000ffc     0 SECTION LOCAL  DEFAULT   17 
    18: 0000000000001058     0 SECTION LOCAL  DEFAULT   18 
    19: 0000000000201d80     0 SECTION LOCAL  DEFAULT   19 
    20: 0000000000201d88     0 SECTION LOCAL  DEFAULT   20 
    21: 0000000000201d90     0 SECTION LOCAL  DEFAULT   21 
    22: 0000000000201f80     0 SECTION LOCAL  DEFAULT   22 
    23: 0000000000202000     0 SECTION LOCAL  DEFAULT   23 
    24: 0000000000202010     0 SECTION LOCAL  DEFAULT   24 
    25: 0000000000000000     0 SECTION LOCAL  DEFAULT   25 
    26: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    27: 0000000000000780     0 FUNC    LOCAL  DEFAULT   14 deregister_tm_clones
    28: 00000000000007c0     0 FUNC    LOCAL  DEFAULT   14 register_tm_clones
    29: 0000000000000810     0 FUNC    LOCAL  DEFAULT   14 __do_global_dtors_aux
    30: 0000000000202010     1 OBJECT  LOCAL  DEFAULT   24 completed.7697
    31: 0000000000201d88     0 OBJECT  LOCAL  DEFAULT   20 __do_global_dtors_aux_fini_array_entry
    32: 0000000000000850     0 FUNC    LOCAL  DEFAULT   14 frame_dummy
    33: 0000000000201d80     0 OBJECT  LOCAL  DEFAULT   19 __frame_dummy_init_array_entry
    34: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS elf3.c
    35: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    36: 00000000000011dc     0 OBJECT  LOCAL  DEFAULT   18 __FRAME_END__
    37: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS 
    38: 0000000000201d88     0 NOTYPE  LOCAL  DEFAULT   19 __init_array_end
    39: 0000000000201d90     0 OBJECT  LOCAL  DEFAULT   21 _DYNAMIC
    40: 0000000000201d80     0 NOTYPE  LOCAL  DEFAULT   19 __init_array_start
    41: 0000000000000ffc     0 NOTYPE  LOCAL  DEFAULT   17 __GNU_EH_FRAME_HDR
    42: 0000000000201f80     0 OBJECT  LOCAL  DEFAULT   22 _GLOBAL_OFFSET_TABLE_
    43: 0000000000000dc0     2 FUNC    GLOBAL DEFAULT   14 __libc_csu_fini
    44: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTable
    45: 0000000000202000     0 NOTYPE  WEAK   DEFAULT   23 data_start
    46: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@@GLIBC_2.2.5
    47: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fread@@GLIBC_2.2.5
    48: 000000000000085a   343 FUNC    GLOBAL DEFAULT   14 print_ehdr
    49: 0000000000202010     0 NOTYPE  GLOBAL DEFAULT   23 _edata
    50: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fclose@@GLIBC_2.2.5
    51: 0000000000000dc4     0 FUNC    GLOBAL DEFAULT   15 _fini
    52: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __stack_chk_fail@@GLIBC_2.4
    53: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@@GLIBC_2.2.5
    54: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@@GLIBC_2.2.5
    55: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND memcmp@@GLIBC_2.2.5
    56: 0000000000202000     0 NOTYPE  GLOBAL DEFAULT   23 __data_start
    57: 00000000000009b1   251 FUNC    GLOBAL DEFAULT   14 print_shdr
    58: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
    59: 0000000000202008     0 OBJECT  GLOBAL HIDDEN    23 __dso_handle
    60: 0000000000000dd0     4 OBJECT  GLOBAL DEFAULT   16 _IO_stdin_used
    61: 0000000000000d50   101 FUNC    GLOBAL DEFAULT   14 __libc_csu_init
    62: 0000000000202018     0 NOTYPE  GLOBAL DEFAULT   24 _end
    63: 0000000000000750    43 FUNC    GLOBAL DEFAULT   14 _start
    64: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fseek@@GLIBC_2.2.5
    65: 0000000000202010     0 NOTYPE  GLOBAL DEFAULT   24 __bss_start
    66: 0000000000000d2e    23 FUNC    GLOBAL DEFAULT   14 main
    67: 0000000000000aac   207 FUNC    GLOBAL DEFAULT   14 print_phdr
    68: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND fopen@@GLIBC_2.2.5
    69: 0000000000000b7b   435 FUNC    GLOBAL DEFAULT   14 read_elf
    70: 0000000000202010     0 OBJECT  GLOBAL HIDDEN    23 __TMC_END__
    71: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
    72: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND __cxa_finalize@@GLIBC_2.2.5
    73: 0000000000000698     0 FUNC    GLOBAL DEFAULT   11 _init

Version symbols section '.gnu.version' contains 14 entries:
 Addr: 00000000000004cc  Offset: 0x0004cc  Link: 5 (.dynsym)
  000:   0 (*local*)       0 (*local*)       2 (GLIBC_2.2.5)   2 (GLIBC_2.2.5)
  004:   2 (GLIBC_2.2.5)   3 (GLIBC_2.4)     2 (GLIBC_2.2.5)   2 (GLIBC_2.2.5)
  008:   2 (GLIBC_2.2.5)   0 (*local*)       2 (GLIBC_2.2.5)   2 (GLIBC_2.2.5)
  00c:   0 (*local*)       2 (GLIBC_2.2.5)

Version needs section '.gnu.version_r' contains 1 entry:
 Addr: 0x00000000000004e8  Offset: 0x0004e8  Link: 6 (.dynstr)
  000000: Version: 1  File: libc.so.6  Cnt: 2
  0x0010:   Name: GLIBC_2.4  Flags: none  Version: 3
  0x0020:   Name: GLIBC_2.2.5  Flags: none  Version: 2

Displaying notes found in: .note.ABI-tag
  Owner                 Data size	Description
  GNU                  0x00000010	NT_GNU_ABI_TAG (ABI version tag)	    OS: Linux, ABI: 3.2.0

Displaying notes found in: .note.gnu.build-id
  Owner                 Data size	Description
  GNU                  0x00000014	NT_GNU_BUILD_ID (unique build ID bitstring)	    Build ID: 6dfc3850dd621c4c2ad0b74e2a137d2089c2b711

```