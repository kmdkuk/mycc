mycc: mycc.c

test: mycc
	./test.sh

clean:
	rm -f mycc *.o *~ tmp*
