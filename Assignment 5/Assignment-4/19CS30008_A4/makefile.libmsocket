CC=gcc
CFLAGS=-Wall

libmsocket.a: mysocket.o
	ar -rcs libmsocket.a mysocket.o

mysocket.o: mysocket.c mysocket.h
	$(CC) $(CFLAGS) -c mysocket.c -lpthread

clean:
	rm -f libmsocket.a mysocket.o
