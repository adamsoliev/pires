CFLAGS=-std=c11 -g -fno-common

luan: main.o
	$(CC) $(CFLAGS) -o luan main.o

test: luan
	./test.sh

clean: 
	rm -f luan *.o *~ tmp*

.PHONY: test clean