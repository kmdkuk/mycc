CC := gcc
CFLAGS := -std=c11

mycc: mycc.c

test: mycc
	./mycc -test
	./test.sh

clean:
	rm -f mycc *.o *~ tmp*
