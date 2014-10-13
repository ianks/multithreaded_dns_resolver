CC = gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -Wall -Wextra

.PHONY: all clean

all: lookup multi_lookup queueTest pthread-hello

lookup: lookup.o queue.o util.o
	$(CC) $(LFLAGS) $^ -o $@

multi_lookup: multi_lookup.o queue.o util.o
	$(CC) $(LFLAGS) $^ -o $@

queueTest: queueTest.o queue.o
	$(CC) $(LFLAGS) $^ -o $@

pthread-hello: pthread-hello.o
	$(CC) $(LFLAGS) $^ -o $@

lookup.o: ref/lookup.c
	$(CC) $(CFLAGS) $<

multi_lookup.o: multi_lookup.c
	$(CC) $(CFLAGS) -pthread $<

queueTest.o: queue/queueTest.c
	$(CC) $(CFLAGS) $<

queue.o: queue/queue.c queue/queue.h
	$(CC) $(CFLAGS) $<

util.o: util/util.c util/util.h
	$(CC) $(CFLAGS) $<

pthread-hello.o: ref/pthread-hello.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f lookup queueTest pthread-hello multi_lookup
	rm -f *.o
	rm -f *~
	rm -f results.txt
