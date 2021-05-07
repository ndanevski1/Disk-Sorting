CC = g++
CCFLAGS = -O3 -Wall -g -ljsoncpp -pthread

all: library.o msort bsort

library.o: library.cc library.h
	$(CC) -o $@ -c $< $(CCFLAGS)

jsoncpp.o: jsoncpp.cpp json/json.h
	$(CC) -o $@ -c $< $(CCFLAGS)

msort: msort.cc library.o
	$(CC) -o $@ $^ $(CCFLAGS)

bsort: bsort.cc
	$(CC) -o $@ $^ $(CCFLAGS)

clean:
	rm -rf *.o msort bsort msort.dSYM bsort.dSYM
