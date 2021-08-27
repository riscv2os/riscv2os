lib_putc (0800080f0)
  0800080f0 0fe010113 addi sp,sp,-32    x[rd:2]=80001fc8
  0800080f4 000812e23 sw s0,28(sp)      x[rd:28]=0
  0800080f8 002010413 addi s0,sp,32     x[rd:8]=80001fe8
  0800080fc 000050793 addi a5,a0,0      x[rd:15]=a
  080008100 0fef407a3 sb a5,-17(s0)     x[rd:15]=a
  080008104 000000013 addi x0,x0,0      x[rd:0]=0
  080008108 0100007b7 lui a5,0x10000000 x[rd:15]=10000000
  08000810c 000578793 addi a5,a5,5      x[rd:15]=10000005
  080008110 00007c783 lbu a5,0(a5)      x[rd:15]=ff
  080008114 00407f793 andi a5,a5,64     x[rd:15]=40
  080008118 0fe0788e3 beq x0,a5,0x80008108      x[rd:17]=0
  08000811c 0100007b7 lui a5,0x10000000 x[rd:15]=10000000
  080008120 0fef44703 lbu a4,-17(s0)    x[rd:14]=a
  080008124 000e78023 sb a4,0(a5)UART: output

        x[rd:0]=0
  080008128 00007c783 lbu a5,0(a5)      x[rd:15]=a
  08000812c 000078513 addi a0,a5,0      x[rd:10]=a
  080008130 001c12403 lw s0,28(sp)      x[rd:8]=80002008
  080008134 002010113 addi sp,sp,32     x[rd:2]=80001fe8
  080008138 000008067 jalr x0,0x8000    x[rd:0]=0
  08000816c 0fec42783 lw a5,-20(s0)     x[rd:15]=800084c3
  080008170 00007c783 lbu a5,0(a5)      x[rd:15]=0
  080008174 0fe0790e3 bne x0,a5,0x80008154      x[rd:1]=8000816c
  080008178 000000013 addi x0,x0,0      x[rd:0]=0
  08000817c 000000013 addi x0,x0,0      x[rd:0]=0
  080008180 001c12083 lw ra,28(sp)      x[rd:1]=80008398
  080008184 001812403 lw s0,24(sp)      x[rd:8]=80002028
  080008188 002010113 addi sp,sp,32     x[rd:2]=80002008
  08000818c 000008067 jalr x0,0x8000    x[rd:0]=0
  080008398 0fec42783 lw a5,-20(s0)     x[rd:15]=0
  08000839c 000178713 addi a4,a5,1      x[rd:14]=1
  0800083a0 000001797 auipc a5,0x1000   x[rd:15]=800093a0
  0800083a4 0c6478793 addi a5,a5,-924   x[rd:15]=80009004
  0800083a8 00007a783 lw a5,0(a5)       x[rd:15]=2
  0800083ac 002f767b3 ?Error:rv_op() funct7=01 not handled!
make: *** [Makefile:28: vrun] Error 1

  需要學 qemu+gdb 對照，否則查不出來！

參考 [RISC-V BYTES: CROSS-PLATFORM DEBUGGING WITH QEMU AND GDB](https://danielmangum.com/posts/risc-v-bytes-qemu-gdb/)

https://stackoverflow.com/questions/55189463/how-to-debug-cross-compiled-qemu-program-with-gdb
