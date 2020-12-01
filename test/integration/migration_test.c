
#include <stdio.h>
#include "test/lsmalloc_test.h"


static inline void *
gc_pmem_append_region(chunk_t *chunk, size_t size)
{
	void *ret = chunk->tail;

	chunk->tail = (void *)((intptr_t)chunk->tail + size);

    chunk->availsize -= size;

	return ret;
}

static inline void
test_region_migration(chunk_t *chunk,chunk_t *gc_chunk,region_t *oldregion)
{   
    //todo region 一个一个移动开销太大,需要改一下索引方式
    region_t *region = (region_t *)malloc(sizeof(region_t));

    region->paddr = gc_pmem_append_region(gc_chunk,oldregion->size);
    region->ptr = oldregion->ptr;
    *region->ptr = (void*)((intptr_t)(region->paddr)+sizeof(pregion_t));
    region->size = oldregion->size;
    region->threadid = oldregion->threadid;
    region->attr = REGION_ALIVE;
    ql_elm_new(region,regions_link);
    ql_tail_insert(&gc_chunk->regions,region,regions_link);
    region->chunk = gc_chunk;
    malloc_printf("Migration: ptr:%p, oldregion:%p, region:%p.\n",region->ptr,oldregion,region);
    ((pregion_t *)(region->paddr))->region = region;
    memcpy((void*)((intptr_t)(region->paddr)+sizeof(pregion_t)),(void*)((intptr_t)(oldregion->paddr)+sizeof(pregion_t)),(oldregion->size-sizeof(pregion_t)));

    return ;
}

chunk_t *
test_alloc_chunk(arena_t *arena,chunk_t *chunk_before){
    void *addr;
    addr = pmempool_chunk_alloc(&arena->pool);

    chunk_t * chunk;
    chunk = malloc(sizeof(chunk_t));

    chunk->paddr = addr;
    ((pchunk_t *)(chunk->paddr))->chunk= chunk;
    chunk->tail = (void*)(((intptr_t)addr)+sizeof(pchunk_t));
    chunk->arena = arena;
    chunk->gcing =false;
    
    chunk->availsize = chunksize-sizeof(pchunk_t);
    chunk->dirtysize = 0;
    chunk->chunktype = CHUNK_TYPE_LOG;
    chunk->live = false;

    malloc_mutex_init(&chunk->gc_lock);
    malloc_mutex_init(&chunk->write_lock);

    ql_new(&chunk->regions);
    sl_elm_new(chunk,avail_link);
    sl_head_insert(&arena->avail_chunks,chunk,avail_link);

    return chunk;

}


int main(){
   
    lspmemdir("/mnt/pmem/");

    void *ptr = lsmalloc(300,&ptr);
    *(int*)ptr = 5;
    malloc_printf("ptr is %p, value is %d\n",ptr,*(int*)ptr);
    region_t *region = ((pregion_t *)((intptr_t)ptr-sizeof(pregion_t)))->region;
    chunk_t *chunk = region->chunk;
    arena_t *arena = chunk->arena;
    chunk_t *gc_chunk = test_alloc_chunk(arena,chunk);
    test_region_migration(chunk,gc_chunk,region);
    malloc_printf("migration ptr is %p, value is %d\n",ptr,*(int*)ptr);
    

    return 0;

}