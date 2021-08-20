CC = gcc
LIB = ../elf/elf.c riscv.c decoder.c csr.c

all: dasm vm

dasm: $(LIB) dasm.c
	$(CC) $^ -o $@

vm: $(LIB) vm.c
	$(CC) $^ -o $@

run:
	dasm ../obj/oshello.elf
#	vm ../obj/oshello.elf

clean:
	rm -f $(TARGET) *.exe