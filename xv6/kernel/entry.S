	# qemu -kernel loads the kernel at 0x80000000
        # and causes each CPU to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text
_entry: # 進入點被設定在 0x80000000
	# set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0    # 設定推疊
        li a0, 1024*4    # 每個核心堆疊大小為 1024 bytes
	csrr a1, mhartid # 取得核心代號 hartid
        addi a1, a1, 1   # a1 = 核心代號+1
        mul a0, a0, a1   # a0 = 4096*a1 
        add sp, sp, a0   # sp = stack0 + (hartid*4096)
	# jump to start() in start.c
        call start       # 呼叫 C 語言的 start 函數
spin:
        j spin           # 這裡不應該被執行，若有就卡住當機
