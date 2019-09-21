CC = gcc
CFLAGS = -Wall -std=c11
SRCS = $(wildcard *.c)
OBJS = $(SRCS: .c=.o)

mycc: $(OBJS)

debug:
	make "CFLAGS = -g -O0"

$(OBJS): mycc.h

test: mycc
	./mycc -test
	./test.sh

clean:
	rm -f mycc *.o *~ tmp*
