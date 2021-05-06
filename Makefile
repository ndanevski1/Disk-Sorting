CC = g++
CCFLAGS = -O3 -Wall -g -ljsoncpp
LEVELDB_DIR = ./leveldb-1.15.0
LEVELDB_OPTS = -I $(LEVELDB_DIR)/include -pthread $(LEVELDB_DIR)/libleveldb.a
JSONCPP_OPTS = -I .

all: library.o msort 

library.o: library.cc library.h
	$(CC) -o $@ -c $< $(CCFLAGS)

parse_schema.o: parse_schema.cc library.h
	$(CC) -o $@ -c $< $(CCFLAGS)

msort: msort.cc library.o parse_schema.o 
	$(CC) -o $@ $^ $(CCFLAGS)

bsort: bsort.cc 
	$(CC) -o $@ $^ $(CCFLAGS) $(LEVELDB_OPTS)
	
clean:
	rm -rf *.o msort bsort msort.dSYM bsort.dSYM
