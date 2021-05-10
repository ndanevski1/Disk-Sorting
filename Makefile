CC = g++
CCFLAGS = -Wall -O3 -g -ljsoncpp -pthread

all: library.o msort bsort

library.o: library.cc library.h
	$(CC) -o $@ -c $< $(CCFLAGS)

parse_schema.o: parse_schema.cc library.h
	$(CC) -o $@ -c $< $(CCFLAGS)

msort: msort.cc RunIterator.cc library.o parse_schema.o 
	$(CC) -o $@ $^ $(CCFLAGS)

bsort: bsort.cc RunIterator.cc library.o parse_schema.o
	$(CC) -o $@ $^ $(CCFLAGS) -lleveldb

clean:
	rm -rf *.o msort bsort msort.dSYM bsort.dSYM
