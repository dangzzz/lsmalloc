#define LSMALLOC_GC_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */
#define GC_THREAD_NUM 8
#define GC_SLOWGC_NUM_PRE_THREAD 3
/******************************************************************************/
/* Function prototypes for non-inline static functions. */

/******************************************************************************/
/* Inline tool function */

/******************************************************************************/

void sum_arena_avail_chunks(arena_t *arena)
{
    int log_sum = 0;
    int gc_sum = 0;
    int fake_sum = 0;
    char types[500] = {0};
    int i = 0;
    chunk_t *chunk = sl_first(&arena->avail_chunks);
    while (chunk != NULL)
    {
        if (chunk->chunktype == CHUNK_TYPE_FAKE)
        {
            types[i] = CHUNK_TYPE_FAKE;
            fake_sum++;
        }
        else if (chunk->chunktype == CHUNK_TYPE_LOG)
        {
            types[i] = CHUNK_TYPE_LOG;
            log_sum++;
        }
        else if (chunk->chunktype == CHUNK_TYPE_GC)
        {
            types[i] = CHUNK_TYPE_GC;
            gc_sum++;
        }
        else
        {
            assert(false);
        }
        i++;
        chunk = sl_next(chunk, avail_link);
    }

    malloc_printf("Current Chunks: Log %d, GC %d, Fake %d.\nChunk List: ", log_sum, gc_sum, fake_sum);
    for (int j = 0; j < i; j++)
    {
        malloc_printf("%c ", types[j]);
        if (j + 1 == i)
        {
            malloc_printf("\n");
        }
    }
    return;
}

static inline void
arena_chunk_dalloc(arena_t *arena, chunk_t *bchunk, chunk_t *chunk)
{
    sl_after_remove(bchunk, chunk, avail_link);
    malloc_printf("Chunk Dalloc: arena is %p, chunk is %p\n", arena, chunk);
    sum_arena_avail_chunks(arena);
    pmempool_free(&pp, chunk);
}

void fastgc_scheduler(void *args)
{
    malloc_printf("FASTGC Begin\n");
    chunk_t *bchunk, *chunk;
    bchunk = sl_first(&((arena_t *)args)->avail_chunks);
    chunk = sl_next(bchunk, avail_link);
    malloc_printf("chunk is %p\n", chunk);
    while (chunk != NULL)
    {
        if ((chunk->live == false) && (chunk->chunktype == CHUNK_TYPE_LOG) && (chunk->availsize + chunk->dirtysize == chunksize - sizeof(pchunk_t)))
        {
            arena_chunk_dalloc((arena_t *)args, bchunk, chunk);
        }
        else
        {
            malloc_printf("Chunk not dead:chunk->live = %d, chunk->chunktype = %c, a+d = %zu, chunksize-head = %zu\n", chunk->live, chunk->chunktype, chunk->availsize + chunk->dirtysize, chunksize - sizeof(chunk_t));
            bchunk = chunk;
        }

        chunk = sl_next(bchunk, avail_link);
    }
}

static inline void
fastgc(arena_t *arena)
{
    threadpool_add(tpool, &fastgc_scheduler, arena, 0);
}

chunk_t *
gc_alloc_chunk(arena_t *arena, chunk_t *chunk_before)
{
    void *addr;
    addr = pmempool_chunk_alloc(&pp);

    chunk_t *chunk;
    chunk = malloc(sizeof(chunk_t));

    chunk->paddr = addr;
    ((pchunk_t *)(chunk->paddr))->chunk = chunk;
    chunk->tail = (void *)(((intptr_t)addr) + sizeof(pchunk_t));
    chunk->arena = arena;
    chunk->gcing = false;

    chunk->availsize = chunksize - sizeof(pchunk_t);
    chunk->dirtysize = 0;
    chunk->chunktype = CHUNK_TYPE_GC;
    chunk->live = false;

    malloc_mutex_init(&chunk->gc_lock);
    malloc_mutex_init(&chunk->write_lock);

    //todo lock
    ql_new(&chunk->regions);
    sl_elm_new(chunk, avail_link);
    sl_head_insert(&arena->avail_chunks, chunk, avail_link);

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

static inline void
gc_region_migration(chunk_t *chunk, chunk_t *gc_chunk, region_t *oldregion)
{
    //todo region 一个一个移动开销太大,需要改一下索引方式
    region_t *region = (region_t *)malloc(sizeof(region_t));

    region->paddr = gc_pmem_append_region(gc_chunk, oldregion->size);
    region->ptr = oldregion->ptr;
    *region->ptr = (void *)((intptr_t)(region->paddr) + sizeof(pregion_t));
    region->size = oldregion->size;
    region->threadid = oldregion->threadid;
    region->attr = REGION_ALIVE;
    ql_elm_new(region, regions_link);
    ql_tail_insert(&gc_chunk->regions, region, regions_link);
    region->chunk = gc_chunk;
    malloc_printf("Migration: ptr:%p, old ptr value:%p, new ptr value:%p.\n", region->ptr, (void *)((intptr_t)(oldregion->paddr) + sizeof(pregion_t)), *region->ptr);
    ((pregion_t *)(region->paddr))->region = region;
    memcpy((void *)((intptr_t)(region->paddr) + sizeof(pregion_t)), (void *)((intptr_t)(oldregion->paddr) + sizeof(pregion_t)), (oldregion->size - sizeof(pregion_t)));

    return;
}

static inline chunk_t *
chunk_do_slowgc(arena_t *arena, chunk_t *chunk, chunk_t *gc_chunk, chunk_t *bchunk, chunk_t *first_chunk)
{

    chunk_t *ret = gc_chunk;
    region_t *region;

    //todo write lock

    ql_foreach(region, &chunk->regions, regions_link)
    {
        if (region->attr == REGION_ALIVE)
        {
            if (gc_chunk->availsize < region->size)
            {
                gc_chunk = gc_alloc_chunk(arena, first_chunk);
                ret = gc_chunk;
            }
            gc_region_migration(chunk, gc_chunk, region);
        }
    }

    //todo write lock

    return ret;
}

void do_own_slowgc(void *args)
{
    slowgc_task_t *task = (slowgc_task_t *)args;
    chunk_t *chunk, *bchunk;
    malloc_printf("Thread %lld:Start own slow gc task,from %p, task %p \n", pthread_self(), task->start, task);
    chunk_t *gc_chunk = gc_alloc_chunk(task->arena, task->first);
    bchunk = task->start;
    chunk = sl_next(bchunk, avail_link);
    for (int i = 0; i < GC_SLOWGC_NUM_PRE_THREAD; i++)
    {
        assert(chunk->live == false);
        assert(chunk->chunktype == CHUNK_TYPE_LOG);
        if ((chunk->availsize + chunk->dirtysize == chunksize - sizeof(pchunk_t)))
        {
            arena_chunk_dalloc(task->arena, bchunk, chunk);
            chunk = sl_next(bchunk, avail_link);
            continue;
        }
        //todo slowgc check
        chunk_t *tmp = chunk_do_slowgc(task->arena, chunk, gc_chunk, bchunk, task->first);
        if (tmp != gc_chunk)
        {
            gc_chunk = tmp;
        }
        arena_chunk_dalloc(task->arena, bchunk, chunk);
        chunk = sl_next(bchunk, avail_link);
    }

    atomic_sub_u(&task->arena->thread_count, 1);

    if (atomic_read_u(&task->arena->thread_count) == 0)
    {
        sem_post(&task->arena->gc_sem);
    }
}

void slowgc_scheduler(void *args)
{
    malloc_printf("--------------------------SLOW GC BEGIN.--------------------------------------\n");
    arena_t *arena = (arena_t *)args;
    chunk_t *first_chunk = sl_first(&arena->avail_chunks);

    chunk_t *bchunk, *gc_chunk;

    arena->thread_count = 1;

    fake_chunk_t *fake_first = malloc(sizeof(fake_chunk_t));
    fake_first->chunktype = CHUNK_TYPE_FAKE;
    sl_elm_new(fake_first, avail_link);
    sl_after_insert(first_chunk, fake_first, avail_link);

    chunk_t *chunk = sl_next(fake_first, avail_link);
    //todo:chunk==NULL?
    fake_chunk_t *start_fchunk = fake_first;
    fake_chunk_t *end_fchunk;
    while (true)
    {
        for (int i = 0; i < GC_SLOWGC_NUM_PRE_THREAD - 1; i++)
        {
            chunk = sl_next(chunk, avail_link);
            if (chunk == NULL)
            {
                goto breakall;
            }
        }
        end_fchunk = malloc(sizeof(fake_chunk_t));
        end_fchunk->chunktype = CHUNK_TYPE_FAKE;
        sl_elm_new(end_fchunk, avail_link);
        sl_after_insert(chunk, end_fchunk, avail_link);
        slowgc_task_t *task = malloc(sizeof(slowgc_task_t));
        task->arena = arena;
        task->start = (chunk_t *)start_fchunk;
        //first no nessary
        task->first = first_chunk;

        atomic_add_u(&arena->thread_count, 1);
        threadpool_add(tpool, &do_own_slowgc, task, 0);

        start_fchunk = end_fchunk;
        chunk = sl_next(start_fchunk, avail_link);
        if (chunk == NULL)
        {
            goto breakall;
        }
    }
breakall:
    //deal with tail chunks less than GC_SLOWGC_NUM_PRE_THREAD.
    bchunk = start_fchunk;
    chunk = sl_next(bchunk, avail_link);
    gc_chunk = gc_alloc_chunk(arena, first_chunk);
    while (chunk != NULL)
    {
        if ((chunk->availsize + chunk->dirtysize == chunksize - sizeof(pchunk_t)))
        {
            arena_chunk_dalloc(arena, bchunk, chunk);
            chunk = sl_next(bchunk, avail_link);
            continue;
        }
        //todo slowgc check
        chunk_t *tmp = chunk_do_slowgc(arena, chunk, gc_chunk, bchunk, first_chunk);
        if (tmp != gc_chunk)
        {
            gc_chunk = tmp;
        }
        arena_chunk_dalloc(arena, bchunk, chunk);
        chunk = sl_next(chunk, avail_link);
    }

    atomic_sub_u(&arena->thread_count, 1);

    if (atomic_read_u(&arena->thread_count) == 0)
    {
        sem_post(&arena->gc_sem);
    }

    sem_wait(&arena->gc_sem);

    sum_arena_avail_chunks(arena);
    /*
    bchunk = first_chunk;
    chunk = sl_next(first_chunk, avail_link);
    while (chunk != NULL)
    {
        if (chunk->chunktype == CHUNK_TYPE_FAKE)
        {
            sl_after_remove(bchunk, chunk, avail_link);
            free(chunk);
        }
        else
        {
            bchunk = chunk;
        }
        chunk = sl_next(first_chunk, avail_link);
    }
    sum_arena_avail_chunks(arena);
*/
//if not debug CHUNK_TYPE_GC is no use
    bchunk = sl_first(&arena->avail_chunks);
    if (bchunk->chunktype == CHUNK_TYPE_GC)
    {
        bchunk->chunktype = CHUNK_TYPE_LOG;
    }
    chunk = sl_next(bchunk, avail_link);
    while (chunk != NULL)
    {
        if (chunk->chunktype == CHUNK_TYPE_GC)
        {
            chunk->chunktype = CHUNK_TYPE_LOG;
            bchunk = sl_next(bchunk, avail_link);
        }
        else if (chunk->chunktype == CHUNK_TYPE_FAKE)
        {
            sl_after_remove(bchunk, chunk, avail_link);
            free(chunk);
        }
        else if(chunk->chunktype == CHUNK_TYPE_LOG){
            bchunk = sl_next(bchunk, avail_link);
        }
        chunk = sl_next(bchunk, avail_link);
    }
    sum_arena_avail_chunks(arena);

    malloc_printf("----------------------SLOW GC END.----------------------------\n");
}

void
slowgc(arena_t *arena)
{
    threadpool_add(tpool, &slowgc_scheduler, arena, 0);
}

void maybe_gc(arena_t *arena)
{

    int usedpct = pmempool_usedpct(&pp);
    if (usedpct >= 70)
    {
        fastgc(arena);
    }

    if (usedpct >= 90)
    {
        slowgc(arena);
    }

    printf("Used %d%% of pool\n", usedpct);

    return;
}
