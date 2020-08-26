#define	LSMALLOC_GC_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */
#define GC_THREAD_NUM   8
/******************************************************************************/
/* Function prototypes for non-inline static functions. */


/******************************************************************************/
/* Inline tool function */


/******************************************************************************/


inline void *
arena_chunk_dalloc(arena_t *arena,chunk_t *chunk)
{   //todo remove whether thread-safe
    ql_remove(&arena->avail_chunks,chunk,avail_link);
    pmempool_free(&arena->pool,chunk);
}

inline void
chunk_do_fastgc(arena_t *arena,chunk_t * chunk){
    arena_chunk_dalloc(arena,chunk);
}


void 
fastgc_scheduler(void *args)
{   
    chunk_t *chunk;
    ql_foreach(chunk,&((arena_t *)args)->avail_chunks,avail_link){
        if((chunk->live==false)&&(chunk->chunktype==CHUNK_TYPE_LOG)&&(chunk->availsize + chunk->dirtysize == chunksize-sizeof(chunk_t))){
            chunk_do_fastgc((arena_t *)args,chunk);
        }
    }
}



static inline void 
fastgc(arena_t *arena)
{
    threadpool_add(tpool,&fastgc_scheduler,arena,0);
}


chunk_t *
gc_alloc_chunk(arena_t *arena){
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
    chunk->chunktype = CHUNK_TYPE_GC;
    chunk->live = false;


    malloc_mutex_init(&chunk->gc_lock);
    malloc_mutex_init(&chunk->write_lock);

    //todo lock
    ql_tail_insert(&arena->avail_chunks,chunk,avail_link);

    return chunk;

}

/* 
 * 追加数据到尾指针所指地址并移动尾指针到数据末端,返回原来尾指针的位置.
 */
static inline void *
gc_pmem_append_region(chunk_t *chunk, size_t size)
{
	void *ret = chunk->tail;

	chunk->tail = (void *)((intptr_t)chunk->tail + size);

    chunk->availsize -= size;

	return ret;
}

inline void
gc_region_migration(chunk_t *chunk,chunk_t *gc_chunk,region_t *oldregion)
{
    region_t *region = (region_t *)malloc(sizeof(region_t));

    region->paddr = gc_pmem_append_region(gc_chunk,oldregion->size);
    ((pregion_t *)(region->paddr))->region = region;
    region->ptr = oldregion->ptr;
    //todo replace ptr
    region->size = oldregion->size;
    region->threadid = oldregion->threadid;
    region->attr = REGION_ALIVE;
    ql_elm_new(region,regions_link);
    ql_tail_insert(&gc_chunk->regions,region,regions_link);
    region->chunk = gc_chunk;
 
    return ;
}


inline chunk_t *
chunk_do_slowgc(arena_t *arena,chunk_t *chunk,chunk_t *gc_chunk)
{
    if((chunk->live==false)&&(chunk->chunktype==CHUNK_TYPE_LOG)&&(chunk->availsize + chunk->dirtysize == chunksize-sizeof(chunk_t))){
            chunk_do_fastgc(arena,chunk);
            return NULL;
    }
    chunk_t *ret = gc_chunk;
    region_t *region;

    malloc_mutex_lock(&chunk->write_lock);

    ql_foreach(region,&chunk->regions,regions_link){
        if(region->attr==REGION_DIRTY){
            if(gc_chunk->availsize<region->size){
                gc_chunk = gc_alloc_chunk(arena);
                ret = gc_chunk;
            }
            gc_region_migration(chunk,gc_chunk,region);
        }
    }

    malloc_mutex_unlock(&chunk->write_lock);

    //todo lock
    ql_remove(&arena->avail_chunks,chunk,avail_link);

    
}



//todo lock-free
void
do_slowgc(void * args)
{
    chunk_t *chunk;
    chunk_t *gc_chunk;
    arena_t *arena = (arena_t *)args;

    gc_chunk = gc_alloc_chunk(arena);

    malloc_mutex_lock(&arena->lock);
    malloc_mutex_lock(&arena->gc_lock);
    chunk = ql_first(&arena->avail_chunks);
    if(chunk == NULL)
        return;
    if(chunk->live==true)
        chunk = ql_next(&arena->avail_chunks,chunk,avail_link);
    malloc_mutex_unlock(&arena->gc_lock);
    malloc_mutex_unlock(&arena->lock);
    if(chunk == NULL)
        return;
    while(true)
    {   
        if(chunk->chunktype == CHUNK_TYPE_GC){
            break;
        }
        malloc_mutex_lock(&arena->gc_lock);
        if(chunk->gcing == true){
            chunk = ql_next(&arena->avail_chunks,chunk,avail_link);  
            continue;
        }else{
            chunk->gcing = true;
        }
        malloc_mutex_unlock(&arena->gc_lock);


        chunk_t *tmp = chunk_do_slowgc(arena,chunk,gc_chunk);
        if(tmp!=gc_chunk){
            gc_chunk = tmp;
        }

        malloc_mutex_lock(&arena->gc_lock);
        ql_remove(&arena->avail_chunks,chunk,avail_link);
        chunk = ql_next(&arena->avail_chunks,chunk,avail_link);
        malloc_mutex_unlock(&arena->gc_lock);  
    }
}


void 
slowgc_scheduler(void *args)
{
    for(int i = 0;i<GC_THREAD_NUM;i++){
        threadpool_add(tpool,&do_slowgc,args,0);
    }

}



static inline void 
slowgc(arena_t *arena){
    threadpool_add(tpool,&slowgc_scheduler,arena,0);
}


void 
maybe_gc(arena_t *arena)
{
    int usedpct = pmempool_usedpct(&arena->pool);
    if(usedpct >= 70){
        fastgc(arena);
    }

    if(usedpct >= 90){
        slowgc(arena);
    }
    
    printf("Used %d\% of pool",usedpct);

    return;
}




