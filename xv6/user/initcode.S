# Initial process that execs /init. (第一個使用者行程 initcode 的組合語言)
# This code runs in user space.
# 可以在編譯完成後用 od -t xC initcode 去印出機器碼，然後放入 proc.c 的 initcode[] 陣列中

#include "syscall.h"

# exec(init, argv)
.globl start
start:
        la a0, init
        la a1, argv
        li a7, SYS_exec
        ecall

# for(;;) exit();
exit:
        li a7, SYS_exit
        ecall
        jal exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0
