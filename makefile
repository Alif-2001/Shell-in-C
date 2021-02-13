CC = gcc
CFLAGS = -std=gnu99 -Wpedantic

all: myShell

myShell: shell.o set1.o set2.o set3.o
	$(CC) $(CFLAGS) shell.o set1.o set2.o set3.o -o myShell

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c -o shell.o

set1.o: set1.c
	$(CC) $(CFLAGS) -c set1.c -o set1.o

set2.o: set2.c
	$(CC) $(CFLAGS) -c set2.c -o set2.o

set3.o: set3.c
	$(CC) $(CFLAGS) -c set3.c -o set3.o

clean:
	rm *o myShell