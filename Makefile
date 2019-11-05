CC=gcc
CFLAGS=-Wall -g -ansi -pedantic -O2  # -S -emit-llvm
LIBS=-L. -lm -lcurl -lpcre -lreadline

all:	sim

sim:	sim.c objgen.c helper.c list.o array.o opcode.c
	$(CC) $(CFLAGS) -o sim sim.c objgen.c helper.c opcode.c list.o array.o

list.o: list.c list.h
	$(CC) -fPIC $(CFLAGS) -c list.c

array.o: array.c array.h
	$(CC) -fPIC $(CFLAGS) -c array.c

clean:
	rm -f *~ *.o sim
