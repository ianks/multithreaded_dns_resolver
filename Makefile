CC = gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -Wall -Wextra -pthread

.PHONY: all clean

all: lookup queueTest pthread-hello

lookup: lookup.o queue.o util.o
	$(CC) $(LFLAGS) $^ -o $@

queueTest: queueTest.o queue.o
	$(CC) $(LFLAGS) $^ -o $@

pthread-hello: pthread-hello.o
	$(CC) $(LFLAGS) $^ -o $@

lookup.o: lookup.c
	$(CC) $(CFLAGS) $<

queueTest.o: queue/queueTest.c
	$(CC) $(CFLAGS) $<

queue.o: queue/queue.c queue/queue.h
	$(CC) $(CFLAGS) $<

util.o: util/util.c util/util.h
	$(CC) $(CFLAGS) $<

pthread-hello.o: pthread-hello.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f lookup queueTest pthread-hello
	rm -f *.o
	rm -f *~
	rm -f results.txt
