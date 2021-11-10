# This Code derived from xv6-riscv (64bit)
# -- https://github.com/mit-pdos/xv6-riscv/blob/riscv/kernel/swtch.S

.globl sys_timer
.align 4
sys_timer:
	call	timer_handler	# call the C timer_handler()
	mret 					# back to interrupt location (pc=mepc)
