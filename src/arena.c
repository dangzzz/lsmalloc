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
    

    return chunk;
}

static inline void *
arena_chunk_alloc_pmem(pmempool_t *pp){
    return pmempool_chunk_alloc(pp);
}

static inline void *
arena_chunk_alloc_vmem(){
    return base_alloc(sizeof(chunk_t));
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

    return false;
}

void * 
arena_malloc_large(arena_t *arena,size_t size, bool zero, void **ptr){
    
}

