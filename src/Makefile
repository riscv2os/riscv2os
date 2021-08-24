CC = gcc
LIB = elf/elf.c riscv/riscv.c riscv/cpu.c riscv/memory.c
ELF = ../data/oshello.elf

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
	vm $(ELF)

clean:
	rm -f $(TARGET) *.exe