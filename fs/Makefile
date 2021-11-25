CC = gcc
CFLAGS = 
all: mkfs

mkfs: mkfs.c ffs.c
	$(CC) $(CFLAGS) -o $@ $^

fs_test: fs_test.c fs.c ffs.c file.c disk.c bio.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.exe
