CC=gcc
CFLAGS=-std=c11 -Wall -g -O0 -static -fno-common -D_GNU_SOURCE
SRCS=$(wildcard *.c)
OBJDIR=./obj
OBJS=$(addprefix $(OBJDIR)/, $(SRCS:.c=.o))
TARGET=bin/mycc

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o  $@ -c $<

test: $(TARGET)
	./test.sh

clean:
	rm -f $(TARGET) $(OBJDIR)/*.o *~ tmp*

.PHONY: test clean
