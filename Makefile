CC := gcc
CFLAGS := -Wall -O3 -I test/include -I include -pthread

.PHONY:test

test:
#编译依赖
	$(CC) $(CFLAGS) -c test/src/test.c -o test/src/test.o
	$(CC) $(CFLAGS) -c test/src/thd.c -o test/src/thd.o
	$(CC) $(CFLAGS) -c src/util.c -o src/util.o
	$(CC) $(CFLAGS) -c src/tsd.c -o src/tsd.o

	$(CC) $(CFLAGS) -c src/mutex.c -o src/mutex.o
	$(CC) $(CFLAGS) -c src/chunk_mmap.c -o src/chunk_mmap.o
	$(CC) $(CFLAGS) -c src/chunk.c -o src/chunk.o
	$(CC) $(CFLAGS) -c src/base.c -o src/base.o
#编译单元测试
	$(CC) $(CFLAGS) test/unit/template.c -o test/unit/template test/src/test.o src/util.o 
	$(CC) $(CFLAGS) test/unit/tsd.c -o test/unit/tsd test/src/test.o src/util.o  src/tsd.o test/src/thd.o
	$(CC) $(CFLAGS) test/unit/base.c -o test/unit/base  test/src/test.o src/util.o  src/base.o src/chunk.o src/chunk_mmap.o src/mutex.o
