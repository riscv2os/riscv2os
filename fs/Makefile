CC = gcc
CFLAGS = 
all: mkfs

mkfs: mkfs.c fs.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.exe
