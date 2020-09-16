#define	LSMALLOC_GC_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */
#define GC_THREAD_NUM   8
#define GC_SLOWGC_NUM_PRE_THREAD 10
/******************************************************************************/
/* Function prototypes for non-inline static functions. */


/******************************************************************************/
/* Inline tool function */


/******************************************************************************/


inline void *
arena_chunk_dalloc(arena_t *arena,chunk_t * bchunk,chunk_t *chunk)
{   
    sl_after_remove(bchunk,chunk,avail_link);
    pmempool_free(&arena->pool,chunk);
}



void 
fastgc_scheduler(void *args)
{   
    chunk_t *bchunk,*chunk;
    bchunk = sl_first(&((arena_t *) args )->avail_chunks);
    chunk = sl_next(bchunk,avail_link);
    while (chunk!=NULL){
        if((chunk->live==false)&&(chunk->chunktype==CHUNK_TYPE_LOG)&&(chunk->availsize + chunk->dirtysize == chunksize-sizeof(chunk_t))){
            arena_chunk_dalloc((arena_t *) args,bchunk,chunk);
        }else{
            bchunk = chunk;
        }
            
        chunk = sl_next(bchunk,avail_link); 
    } 
}



static inline void 
fastgc(arena_t *arena)
{
    threadpool_add(tpool,&fastgc_scheduler,arena,0);
}


chunk_t *
gc_alloc_chunk(arena_t *arena,chunk_t *chunk_before){
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

    //todo lock
    sl_elm_new(chunk,avail_link);
    sl_after_insert(chunk_before,chunk,avail_link);
    return chunk;

}


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
    //todo region 一个一个移动开销太大,需要改一下索引方式
    region_t *region = (region_t *)malloc(sizeof(region_t));

    region->paddr = gc_pmem_append_region(gc_chunk,oldregion->size);
    ((pregion_t *)(region->paddr))->region = region;
    region->ptr = oldregion->ptr;
    *region->ptr = (void*)((intptr_t)(region->paddr)+sizeof(pregion_t));
    region->size = oldregion->size;
    region->threadid = oldregion->threadid;
    region->attr = REGION_ALIVE;
    ql_elm_new(region,regions_link);
    ql_tail_insert(&gc_chunk->regions,region,regions_link);
    region->chunk = gc_chunk;
    return ;
}

inline chunk_t *
chunk_do_slowgc(arena_t *arena,chunk_t *chunk,chunk_t *gc_chunk,chunk_t *bchunk,chunk_t *first_chunk)
{   

    chunk_t *ret = gc_chunk;
    region_t *region;

    //todo write lock

    ql_foreach(region,&chunk->regions,regions_link){
        if(region->attr==REGION_DIRTY){
            if(gc_chunk->availsize<region->size){
                gc_chunk = gc_alloc_chunk(arena,first_chunk);
                ret = gc_chunk;
            }
            gc_region_migration(chunk,gc_chunk,region);
        }
    }

    //todo write lock



    
}



void
do_own_slowgc(void *args)
{
    slowgc_task_t *task = (slowgc_task_t *)args;
    chunk_t *chunk,*bchunk;
    chunk_t *gc_chunk = gc_alloc_chunk(task->arena,task->first);
    bchunk = task->start;
    chunk  = sl_next(bchunk,avail_link);
    for(int i = 0;i<GC_SLOWGC_NUM_PRE_THREAD;i++)
    {   
        assert(chunk->live==false);
        assert(chunk->chunktype==CHUNK_TYPE_LOG);
        if((chunk->availsize + chunk->dirtysize == chunksize-sizeof(chunk_t))){
            arena_chunk_dalloc(task->arena,bchunk,chunk);
            chunk = sl_next(bchunk,avail_link);
            continue;
        }
        //todo slowgc check
        chunk_t *tmp = chunk_do_slowgc(task->arena,chunk,gc_chunk,bchunk,task->first);
        if(tmp!=gc_chunk){
            gc_chunk = tmp;
        }
        arena_chunk_dalloc(task->arena,bchunk,chunk);
        chunk = sl_next(bchunk,avail_link);

    }

    atomic_sub_u(&task->arena->thread_count,1);

    if(atomic_read_u(&task->arena->thread_count)==0){
        sem_post(&task->arena->gc_sem);
    }
    
}



void 
slowgc_scheduler(void *args)
{   
    arena_t *arena = (arena_t *) args;
    chunk_t *first_chunk = sl_first(&arena->avail_chunks);

    arena->thread_count = 1;

    fake_chunk_t *fake_first = malloc(sizeof(fake_chunk_t));
    sl_elm_new(fake_first,avail_link);
    sl_after_insert(first_chunk,fake_first,avail_link);


    chunk_t *chunk = sl_next(fake_first,avail_link);
    //todo:chunk==NULL?
    fake_chunk_t *start_fchunk = fake_first;
    fake_chunk_t *end_fchunk;
    while(true){
        for(int i = 0;i < GC_SLOWGC_NUM_PRE_THREAD-1;i++){
            chunk = sl_next(chunk,avail_link);
            if(chunk==NULL){
                goto breakall;
            }
        }
        end_fchunk = malloc(sizeof(fake_chunk_t));
        sl_elm_new(end_fchunk,avail_link);
        sl_after_insert(chunk,end_fchunk,avail_link);
        slowgc_task_t *task = malloc(sizeof(slowgc_task_t));
        task->arena = arena;
        task->start = start_fchunk;
        //first no nessary
        task->first = first_chunk;

        atomic_add_u(&arena->thread_count,1);
        threadpool_add(tpool,&do_own_slowgc,task,0);

        start_fchunk = end_fchunk;
        chunk = sl_next(start_fchunk,avail_link);
        if(chunk==NULL){
            goto breakall;
        }
    }
breakall:
    //deal with tail chunks less than GC_SLOWGC_NUM_PRE_THREAD.
    chunk_t *bchunk = start_fchunk;
    chunk = sl_next(bchunk,avail_link);
    chunk_t *gc_chunk = gc_alloc_chunk(arena,first_chunk);
    while(chunk!=NULL){
        if((chunk->availsize + chunk->dirtysize == chunksize-sizeof(chunk_t))){
            arena_chunk_dalloc(arena,bchunk,chunk);
            chunk = sl_next(bchunk,avail_link);
            continue;
        }
         //todo slowgc check
        chunk_t *tmp = chunk_do_slowgc(arena,chunk,gc_chunk,bchunk,first_chunk);
        if(tmp!=gc_chunk){
            gc_chunk = tmp;
        }
        arena_chunk_dalloc(arena,bchunk,chunk);
        chunk = sl_next(chunk,avail_link);
    }

    atomic_sub_u(arena->thread_count,1);

    if(atomic_read_u(arena->thread_count)==0){
        sem_post(&arena->gc_sem);
    }

    sem_wait(&arena->gc_sem);
    //delete fake

    

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




