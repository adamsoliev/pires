CFLAGS=-std=c11 -g -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

luan: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

$(OBJS): luan.h

test: luan
	./test.sh

clean: 
	rm -f luan *.o *~ tmp*

.PHONY: test clean