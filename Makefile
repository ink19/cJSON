CC=gcc

CFLAGS = -g
LDFLAGS = -lm

test: test.o cjson.o
	$(CC) -o test test.o cjson.o $(LDFLAGS)

cjson.o: cjson.c cjson.h

test.o: test.c cjson.h

clean:
	rm -rf *.o test