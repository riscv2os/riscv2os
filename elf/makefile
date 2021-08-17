CC = gcc
TARGET = elfdump

all: $(TARGET) run

elfdump: elf.c $(TARGET).c
	$(CC) $^ -o $@

run: $(TARGET)
	$(TARGET) ../obj/oshello.elf

clean:
	rm -f $(TARGET) *.exe