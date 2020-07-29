CC := gcc
CFLAGS := -Wall -O3 -I test/include -I include

.PHONY:test

test:
	$(CC) $(CFLAGS) -c test/src/test.c -o test/src/test.o
	$(CC) $(CFLAGS) -c test/src/thd.c -o test/src/thd.o
	$(CC) $(CFLAGS) -c src/util.c -o src/util.o
	$(CC) $(CFLAGS) test/unit/template.c -o test/unit/template test/src/test.o src/util.o 
	