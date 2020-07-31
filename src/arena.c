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
    arena_maxlarge = 0;
    arena_maxsmall = 0;
}


static chunk_t *
arena_chunk_alloc(arena_t *arena){
    void *addr;
    addr = arena_chunk_alloc_pmem(&arena->pool);

    chunk_t * chunk;
    chunk = arena_chunk_alloc_vmem();
    chunk->paddr = addr;
    chunk->tail = addr;
    chunk->arena = arena;
    ql_new(&chunk->regions);
    chunk->availsize = chunksize;

    ql_elm_new(chunk,avail_link);
    ql_head_insert(&arena->avail_chunks,chunk,avail_link);
    return chunk;
}

static inline void *
arena_chunk_alloc_pmem(pmempool_t *pp){
    return pmempool_chunk_alloc(pp);
}

static inline void *
arena_chunk_alloc_vmem(){
    return  malloc(sizeof(chunk_t));
}

bool
arena_new(arena_t *arena, unsigned ind)
{
	unsigned i;

    if (malloc_mutex_init(&arena->lock))
	    return (true);
	arena->ind = ind;
	arena->nthreads = 0;
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

static void 
arena_sort_avail_chunks(arena_t *arena,chunk_t *chunk){

}

static void *
arena_region_alloc(arena_t *arena,size_t size, bool zero, void **ptr)
{
    chunk_t *chunk;
    region_t *region = (region_t *)malloc(sizeof(region_t));
    chunk=ql_first(&arena->avail_chunks);
    if(chunk->availsize<size){
        chunk = arena_chunk_alloc(arena);
    }
    region->paddr = arena_pmem_append_region(arena,chunk,size);
    region->ptr = ptr;
    region->size = size;
    region->threadid = lid;
    region->attr = REGION_ALIVE;
    ql_elm_new(region,regions_link);
    ql_tail_insert(&chunk->regions,region,regions_link);

    arena_sort_avail_chunks(arena,chunk);

    return region->paddr;
}

void * 
arena_malloc_large(arena_t *arena,size_t size, bool zero, void **ptr)
{
    void        *ret;

    malloc_mutex_lock(&arena->lock);

    /* 分配的大小对齐到8字节 */
    size += sizeof(pregion_t);
	size = ALIGNMENT_CEILING(size, sizeof(long long));
    
    ret = arena_region_alloc(arena,size,zero,ptr);

    return ret;

	malloc_mutex_unlock(&arena->lock);
}

