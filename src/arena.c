#define	LSMALLOC_ARENA_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */

size_t      arena_maxlarge;
size_t      arena_maxsmall;

/******************************************************************************/
/* Function prototypes for non-inline static functions. */


/******************************************************************************/
/* Inline tool function */


/******************************************************************************/


void
arena_boot(void)
{
    //todo
    arena_maxlarge = chunksize;
    arena_maxsmall = 0;
}



static inline void *
arena_chunk_alloc_pmem(pmempool_t *pp){
    return pmempool_chunk_alloc(pp);
}


static chunk_t *
arena_chunk_alloc(arena_t *arena){
    chunk_t * chunk = arena_chunk_alloc_pmem(&arena->pool);

    chunk->tail = (void*)(((intptr_t)chunk)+sizeof(chunk_t));
    chunk->arena = arena;
    ql_new(&chunk->regions);
    chunk->availsize = chunksize-sizeof(chunk_t);
    chunk->chunktype = CHUNK_TYPE_LOG;

    ql_elm_new(chunk,avail_link);
    ql_head_insert(&arena->avail_chunks,chunk,avail_link);
    return chunk;
}



bool
arena_new(arena_t *arena, unsigned ind)
{

    if (malloc_mutex_init(&arena->lock))
	    return (true);
	arena->ind = ind;
	arena->nthreads = 0;
    arena->maxchunk = NULL;
    pmempool_create(&arena->pool);

    ql_new(&arena->avail_chunks);
    return false;
}


/* 
 * 追加数据到尾指针所指地址并移动尾指针到数据末端,返回原来尾指针的位置.
 */
static inline void *
arena_pmem_append_region(arena_t *arena, chunk_t *chunk, size_t size)
{
	void *ret = chunk->tail;

	chunk->tail = (void *)((intptr_t)chunk->tail + size);

    chunk->availsize -= size;

	return ret;
}



static void *
arena_region_alloc(arena_t *arena,size_t size, bool zero, void **ptr)
{
    chunk_t *chunk;
    
    chunk=ql_first(&arena->avail_chunks);
    if(chunk == NULL||chunk->availsize<size){
        chunk = arena_chunk_alloc(arena);
    }
    region_t *region = arena_pmem_append_region(arena,chunk,size);
    region->ptr = ptr;
    region->size = size;
    region->threadid = *lid_tsd_get();
    region->attr = REGION_ALIVE;
    ql_elm_new(region,regions_link);
    ql_tail_insert(&chunk->regions,region,regions_link);
    region->chunk = chunk;
 
    return (void*)((intptr_t)(region)+sizeof(region_t));
}

void * 
arena_malloc_large(arena_t *arena,size_t size, bool zero, void **ptr)
{
    void        *ret;

    malloc_mutex_lock(&arena->lock);

    /* 分配的大小对齐到8字节 */
    size += sizeof(region_t);
	size = ALIGNMENT_CEILING(size, sizeof(long long));
    
    ret = arena_region_alloc(arena,size,zero,ptr);
    
    malloc_mutex_unlock(&arena->lock);
    return ret;

	
}


void 
arena_dalloc_large_locked(arena_t *arena,chunk_t *chunk,region_t *region)
{
    region->attr = REGION_DIRTY;
}

void 
arena_dalloc_large(arena_t *arena,chunk_t *chunk,region_t *region)
{
    malloc_mutex_lock(&arena->lock);
	arena_dalloc_large_locked(arena, chunk, region);
	malloc_mutex_unlock(&arena->lock);

}

