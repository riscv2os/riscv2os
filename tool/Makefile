CC = gcc
LIB = ../elf/elf.c ../riscv/riscv.c ../riscv/cpu.c

all: elfdump dasm

elfdump: $(LIB) elfdump.c
	$(CC) $^ -o $@

dasm: $(LIB) dasm.c
	$(CC) $^ -o $@

erun:
	elfdump ../data/oshello.elf

drun:
	dasm ../data/oshello.elf

clean:
	rm -f $(TARGET) *.exe