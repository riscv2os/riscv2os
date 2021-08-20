CC = gcc
LIB = elf/elf.c riscv/riscv.c riscv/cpu.c
ELF = ../data/oshello.elf

all: elfdump dasm

elfdump: $(LIB) elfdump.c
	$(CC) $^ -o $@

dasm: $(LIB) dasm.c
	$(CC) $^ -o $@

erun:
	elfdump $(ELF)

drun:
	dasm $(ELF)

clean:
	rm -f $(TARGET) *.exe