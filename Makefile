CC=gcc
CFLAGS=-std=c11 -g -static -fno-common -D_GNU_SOURCE
SRCS=$(wildcard *.c)
OBJDIR=./obj
OBJS=$(addprefix $(OBJDIR)/, $(SRCS:.c=.o))
TARGET=bin/mycc

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	$(CC) -o  $@ -c $<

test: $(TARGET)
	./test.sh

clean:
	rm -f $(TARGET) $(OBJDIR)/*.o *~ tmp*

.PHONY: test clean
