CC = gcc
LIB = elf/elf.c riscv/riscv.c riscv/cpu.c riscv/memory.c lib/c_map.c elf/sym.c
# ELF = ../data/os_hello.elf // success
# ELF = ../data/os_context_switch.elf // success
ELF = ../data/os_multitasking.elf
# ELF = ../data/hello.elf // fail , because hello.elf is 16bits.

all: elfdump dasm

elfdump: $(LIB) elfdump.c
	$(CC) $^ -o $@

dasm: $(LIB) dasm.c
	$(CC) $^ -o $@

vm: $(LIB) vm.c
	$(CC) $^ -o $@

erun:
	elfdump $(ELF)

drun:
	dasm $(ELF)

vrun:
#	vm $(ELF)
	vm $(ELF) -t

clean:
	rm -f $(TARGET) *.exe