CFLAGS=-std=c++2a -g -static -fno-common
CC=g++
SRCS=$(wildcard src/*.cc)
OBJS=$(SRCS:.c=.o)

cxxchibicc: $(OBJS)
	$(CC) $(CFLAGS) -o  $@ $^ $(CFLAGS)

$(OBJS): src/cxxchibicc.h


test: cxxchibicc
	./test.sh

clean:
	rm -r cxxchibicc *.o *~ tmp*

.PHONY: test clean