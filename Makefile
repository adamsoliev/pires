CFLAGS=-std=c11 -g -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

pires: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

$(OBJS): pires.h

test: pires
	./test.sh

clean: 
	rm -f pires *.o *~ tmp*

.PHONY: test clean