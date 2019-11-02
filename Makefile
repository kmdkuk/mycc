CC=gcc
CFLAGS=-std=c11 -g -static -fno-common -D_GNU_SOURCE
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mycc: $(OBJS)
	$(CC) -o mycc $(OBJS) $(LDFLAGS)

$(OBJS): mycc.h

test: mycc
	./test.sh

clean:
	rm -f mycc *.o *~ tmp*

.PHONY: test clean
