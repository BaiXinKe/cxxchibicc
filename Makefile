CFLAGS=-std=c++2a -g -static -fno-common
CC=g++

cxxchibicc: main.o
	$(CC) -o $@ src/main.cc $(LDFLAGS)

main.o: src/main.cc
	$(CC) -o $@ src/main.cc $(LDFLAGS)

test: cxxchibicc
	./test.sh

clean:
	rm -r cxxchibicc *.o *~ tmp*

.PHONY: test clean