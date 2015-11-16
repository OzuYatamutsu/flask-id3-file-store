CC=gcc
CFLAGS=-Wall -D_FILE_OFFSET_BITS=64 -I -g
PROG=yourtuneslib
FUSE_DEPS=`pkg-config fuse --cflags --libs`

all: $(PROG).o
	$(CC) $(CFLAGS) *.c $(FUSE_DEPS) -o $(PROG)

clean:
	rm -fv $(PROG) *.o
