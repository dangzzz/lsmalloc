
#include <stdio.h>
#include "test/lsmalloc_test.h"
#include <stdlib.h>
#define FREED 0xff
//datasize 整除sumsize ptrs大小为商

void malloc_large_data(size_t datasize, size_t sumsize, void **ptrs)
{
    for (int i = 0; i < sumsize; i += datasize)
    {
        ptrs[i / datasize]=lsmalloc(datasize, &ptrs[i / datasize]);
    }
    return;
}

void free_large_data(size_t datasize, size_t sumsize, void **ptrs)
{
    for (int i = 0; i < sumsize / datasize; i++)
    {

        lsfree(ptrs[i]);
    }
    return;
}

void free_large_data_random(size_t datasize, size_t sumsize, void **ptrs)
{
    for (int freesize = 0; freesize < sumsize; freesize += datasize)
    {
        int ran = rand() % (sumsize / datasize);
        while ((intptr_t)ptrs[ran] == FREED)
        {
            ran = rand() % (sumsize / datasize);
        }
        lsfree(ptrs[ran]);
        ptrs[ran] = (void *)FREED;
    }
    return;
}

int main()
{
    size_t datasize = 1024;
    size_t sumsize = 1024 * 1024 * 4 * 10;
    void **ptrs = malloc(327680UL);
    malloc_large_data(datasize, sumsize, ptrs);
    free_large_data(datasize, sumsize, ptrs);
    void *ptr = lsmalloc(300, &ptr);
    region_t *region = ((pregion_t *)((intptr_t)ptr - sizeof(pregion_t)))->region;
    chunk_t *chunk = region->chunk;
    arena_t *arena = chunk->arena;
    maybe_gc(arena);
    getchar();
    return 0;
}