CC = gcc
CFLAGS = -Wall -std=c11
SRCS = $(wildcard *.c)
OBJS = $(SRCS: .c=.o)

mycc: $(OBJS)

$(OBJS): mycc.h

test: mycc
	./mycc -test
	./test.sh

clean:
	rm -f mycc *.o *~ tmp*
