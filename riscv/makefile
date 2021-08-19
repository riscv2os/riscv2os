CC = gcc
TARGET = ./dasm
LIB = ../elf/elf.c riscv.c decoder.c

all: $(TARGET)

$(TARGET): $(LIB) $(TARGET).c
	$(CC) $^ -o $@

run: $(TARGET)
	$(TARGET) ../obj/oshello.elf

clean:
	rm -f $(TARGET) *.exe