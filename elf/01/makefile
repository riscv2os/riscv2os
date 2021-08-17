all: elfdump run

elfdump: elf.c elfdump.c 
	gcc $^ -o $@

run:
	./elfdump oshello.elf
