CC ?= gcc
CFLAGS = -O2 -Wall
LDFLAGS = -lpthread

all:
	$(CC) $(CFLAGS) -o semu semu.c $(LDFLAGS)

kernel.bin:
	scripts/download.sh

check: all kernel.bin
	./semu kernel.bin fs.img

clean:
	rm -f semu
distclean: clean
	rm -f kernel.bin fs.img
