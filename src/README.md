# riscv

最近總是偷學 github 上的程式碼！

我把 jserv 的 rvemu32-next 讀懂之後，改寫成三個應用

1. elfdump -- 印出 elf 檔案的內容，類似 objdump
2. dasm -- RISC-V 32bits 的反組譯器，會讀入 ELF 然後反組譯出程式段。
3. vm -- RISC-V 32bits 的虛擬機，會讀入 ELF 然後像 QEMU 那樣執行該程式。

## vm

```
$ make vm
gcc elf/elf.c riscv/riscv.c riscv/cpu.c riscv/memory.c vm.c -o vm
$ make vrun
vm ../data/oshello.elf
Hello OS!
```

case 2:

```
$ make vm
gcc elf/elf.c riscv/riscv.c riscv/cpu.c riscv/memory.c lib/c_map.c elf/sym.c vm.c -o vm
$ make vrun
vm ../data/os_multitasking_myvm.elf
OS start
OS: Activate next task
Task0: Created!
Task0: Now, return to kernel mode
OS: Back to OS

OS: Activate next task
Task1: Created!
Task1: Now, return to kernel mode
OS: Back to OS

OS: Activate next task
Task0: Running...
OS: Back to OS

OS: Activate next task
Task1: Running...
OS: Back to OS

OS: Activate next task
Task0: Running...
OS: Back to OS

OS: Activate next task
Task1: Running...
OS: Back to OS

OS: Activate next task
Task0: Running...
OS: Back to OS

OS: Activate next task
Task1: Running...
make: *** [Makefile:28: vrun] Interrupt
```

## dasm

```
$ make run
gcc ../elf/elf.c riscv.c dasm.c -o dasm
./dasm ../obj/oshello.elf
s.body=0000000000412988 offset=0x80000000 size=32996
080000000: f14022f3 csrrs t0,x0,mhartid
080000004: 00a29293 slli t0,t0,10
080000008: 00002117 auipc sp,0x2
08000000c: 02010113 addi sp,sp,32
080000010: 00510133 add sp,sp,t0
080000014: f1402573 csrrs a0,x0,mhartid
080000018: 00051463 bne x0,a0,0x80000020
08000001c: 0a80806f jal x0,0x800080c4
080000020: 10500073 wfi
080000024: ffdff06f jal x0,0x80000020
080008028: fe010113 addi sp,sp,-32
08000802c: 00812e23 sw s0,28(sp)
080008030: 02010413 addi s0,sp,32
080008034: 00050793 addi a5,a0,0
080008038: fef407a3 sb a5,-17(s0)
08000803c: 00000013 addi x0,x0,0
080008040: 100007b7 lui a5,0x10000
080008044: 00578793 addi a5,a5,5
080008048: 0007c783 lbu a5,0(a5)
08000804c: 0407f793 andi a5,a5,64
080008050: fe0788e3 beq x0,a5,0x80008040
080008054: 100007b7 lui a5,0x10000
080008058: fef44703 lbu a4,-17(s0)
08000805c: 00e78023 sb a4,0(a5)
080008060: 0007c783 lbu a5,0(a5)
080008064: 00078513 addi a0,a5,0
080008068: 01c12403 lw s0,28(sp)
08000806c: 02010113 addi sp,sp,32
080008070: 00008067 jalr x0,0x8
080008074: fe010113 addi sp,sp,-32
080008078: 00112e23 sw ra,28(sp)
08000807c: 00812c23 sw s0,24(sp)
080008080: 02010413 addi s0,sp,32
080008084: fea42623 sw a0,-20(s0)
080008088: 01c0006f jal x0,0x800080a4
08000808c: fec42783 lw a5,-20(s0)
080008090: 00178713 addi a4,a5,1
080008094: fee42623 sw a4,-20(s0)
080008098: 0007c783 lbu a5,0(a5)
08000809c: 00078513 addi a0,a5,0
0800080a0: f89ff0ef jal ra,0x80008028
0800080a4: fec42783 lw a5,-20(s0)
0800080a8: 0007c783 lbu a5,0(a5)
0800080ac: fe0790e3 bne x0,a5,0x8000808c
0800080b0: 00000013 addi x0,x0,0
0800080b4: 01c12083 lw ra,28(sp)
0800080b8: 01812403 lw s0,24(sp)
0800080bc: 02010113 addi sp,sp,32
0800080c0: 00008067 jalr x0,0x8
0800080c4: ff010113 addi sp,sp,-16
0800080c8: 00112623 sw ra,12(sp)
0800080cc: 00812423 sw s0,8(sp)
0800080d0: 01010413 addi s0,sp,16
0800080d4: 00000517 auipc a0,0x0
0800080d8: 01050513 addi a0,a0,16
0800080dc: f99ff0ef jal ra,0x80008074
0800080e0: 0000006f jal x0,0x800080e0
```

## elfdump

```
$ make elfdump
gcc elf/elf.c riscv/riscv.c riscv/cpu.c riscv/memory.c elfdump.c -o elfdump
$ make erun
elfdump ../data/oshello.elf
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


.text
fffffff3 22 40 fffffff1 ffffff93 ffffff92 ffffffa2 00 17 21 00 00 13 01 01 02 33 01 51 00 73 25 40 fffffff1 63 14 05 00 6f ffffff80 ffffff80 0a 73 00 50 10 6f fffffff0 ffffffdf ffffffff 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 ...
.rodata
48 65 6c 6c 6f 20 4f 53 21 0a 00
.data
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ...
.riscv.attributes
A%   riscv v32i2p0_m2p0_a2p0
.comment
47 43 43 3a 20 28 53 69 46 69 76 65 20 47 43 43 20 38 2e 33 2e 30 2d 32 30 32 30 2e 30 34 2e 31 29 20 38 2e 33 2e 30 00
.symtab
  name:           section:           value:0x0          size:0     info:(local,none)
  name:           section:.text      value:0x80000000   size:0     info:(local,section)
  name:           section:.rodata    value:0x800080e4   size:0     info:(local,section)
  name:           section:.data      value:0x800080ef   size:0     info:(local,section)
  name:           section:.riscv.attributes value:0x0          size:0     info:(local,section)
  name:           section:.comment   value:0x0          size:0     info:(local,section)
  name:C:\Users\user\AppData\Local\Temp\ccNpw8Ob.o section:?          value:0x0          size:0     info:(local,file)
  name:STACK_SIZE section:?          value:0x2000       size:0     info:(local,none)
  name:stacks     section:.text      value:0x80000028   size:0     info:(local,none)
  name:park       section:.text      value:0x80000020   size:0     info:(local,none)
  name:os.c       section:?          value:0x0          size:0     info:(local,file)
  name:lib_putc   section:.text      value:0x80008028   size:76    info:(global,func)
  name:lib_puts   section:.text      value:0x80008074   size:80    info:(global,func)
  name:os_main    section:.text      value:0x800080c4   size:32    info:(global,func)
  name:_start     section:.text      value:0x80000000   size:0     info:(global,none)

.strtab
 C:\Users\user\AppData\Local\Temp\ccNpw8Ob.o STACK_SIZE stacks park os.c lib_putc lib_puts os_main _start 
.shstrtab
 .symtab .strtab .shstrtab .text .rodata .data .riscv.attributes .comment 
```
