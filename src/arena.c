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
    arena_maxsmall = 224; //暂定
}



static inline void *
arena_chunk_alloc_pmem(pmempool_t *pp){
    return pmempool_chunk_alloc(pp);
}

static inline void *
arena_chunk_alloc_vmem(){
    return  malloc(sizeof(chunk_t));
}

static chunk_t *
arena_chunk_alloc(arena_t *arena, char chunktype){
    void *addr;
    addr = arena_chunk_alloc_pmem(&arena->pool);

    chunk_t * chunk;
    chunk = arena_chunk_alloc_vmem();


    chunk->paddr = addr;
    ((pchunk_t *)(chunk->paddr))->chunk= chunk;
    chunk->tail = (void*)(((intptr_t)addr)+sizeof(pchunk_t));
    chunk->arena = arena;
    
    chunk->availsize = chunksize-sizeof(pchunk_t);
    chunk->chunktype = chunktype;

    if (chunktype == CHUNK_TYPE_LOG)
    {
        ql_new(&chunk->regions);
        ql_elm_new(chunk,avail_link);
        ql_head_insert(&arena->avail_chunks,chunk,avail_link);
    }
    else  //插入尾部
    {
        ql_elm_new(chunk,avail_link);
        ql_tail_insert(&arena->avail_chunks,chunk,avail_link);
    }

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

    int i;
    for (i = 0; i < CLASS_NUM; i = i+1)
    {
        arena->avail_sregion[i] = NULL;
    }
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
    region_t *region = (region_t *)malloc(sizeof(region_t));
    chunk=ql_first(&arena->avail_chunks);
    if(chunk == NULL||chunk->availsize<size||chunk->chunktype!=CHUNK_TYPE_LOG){
        chunk = arena_chunk_alloc(arena, CHUNK_TYPE_LOG);
    }
    region->paddr = arena_pmem_append_region(arena,chunk,size);
    ((pregion_t *)(region->paddr))->region = region;
    region->ptr = ptr;
    region->size = size;
    region->threadid = *lid_tsd_get();
    region->attr = REGION_ALIVE;
    ql_elm_new(region,regions_link);
    ql_tail_insert(&chunk->regions,region,regions_link);
    region->chunk = chunk;
 
    return (void*)((intptr_t)(region->paddr)+sizeof(pregion_t));
}

void * 
arena_malloc_large(arena_t *arena,size_t size, bool zero, void **ptr)
{
 //   printf("large\n");
 
    void        *ret;

    malloc_mutex_lock(&arena->lock);

    /* 分配的大小对齐到8字节 */
    size += sizeof(pregion_t);
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


/*以下是small类型的malloc和free*/



void *
arena_malloc_small_hard(arena_t *arena,size_t size, bool zero, void **ptr, unsigned short cls)
{
 //   printf("small hard\n");

    chunk_t *chunk = ql_last(&arena->avail_chunks, avail_link); //从尾部取

    //chunk_t *chunk = ql_first(&arena->avail_chunks); 
    sregion_t *sregion = (sregion_t *)malloc(sizeof(sregion_t));
    void * ret;

    if(chunk == NULL||chunk->availsize < sregion_size||chunk->chunktype!=CHUNK_TYPE_SLAB){
        chunk = arena_chunk_alloc(arena, CHUNK_TYPE_SLAB);
    }

    sregion->paddr = chunk->tail;
    chunk->tail = (void*)((uintptr_t)chunk->tail + sregion_size);
    chunk->availsize -= sregion_size;

    ((psregion_t *)(sregion->paddr))->sregion = sregion;
    void * tail = (void*)((uintptr_t)sregion->paddr + sizeof(psregion_t));
    ret = (void *)((uintptr_t)tail+sizeof(pslab_t));

    sregion->size = size;
    sregion->ptail = (void *)((uintptr_t)ret + size);
    arena->avail_sregion[cls] = sregion;
 
    ((pslab_t *)tail)->ptr = ptr;
    ((pslab_t *)tail)->attr = 'c';

    return ret;
}

void *
arena_malloc_small_easy
(arena_t *arena,size_t size, bool zero, void **ptr, sregion_t * sregion, unsigned short cls)
{
  // printf("small easy\n");

    void * ret;
    assert(sregion != NULL);
    
    void * tail = (void *)((uintptr_t)sregion->ptail+sizeof(pslab_t)+size);
    if ((uintptr_t)tail > (uintptr_t)sregion->paddr+sregion_size)
    {
        return arena_malloc_small_hard(arena, size, zero, ptr, cls);
    }
    else
    {
        void *tail = sregion->ptail;
        ret = (void *)((uintptr_t)tail+sizeof(pslab_t));

        ((pslab_t *)tail)->ptr = ptr;
        ((pslab_t *)tail)->attr = 'c';
        sregion->ptail = (void *)((uintptr_t)ret+size);

        return ret;
    }   
}

/*
分配时，先看arena->sregion_avail[size]。如果这个能分配则直接分配；如果不能，则hard分配
hard分配：需要分配一个新的sregion
*/
void * 
arena_malloc_small(arena_t *arena,size_t size, bool zero, void **ptr)
{
    void        *ret;

    malloc_mutex_lock(&arena->lock);

    unsigned short cls = size_to_class(size);
    size = class_to_size(cls);
    sregion_t * sregion = arena->avail_sregion[cls];
    if (sregion != NULL)
    {
        ret = arena_malloc_small_easy(arena, size, zero, ptr, sregion, cls);
    }
    else
    {
        ret = arena_malloc_small_hard(arena, size, zero, ptr, cls);
    }
    
    malloc_mutex_unlock(&arena->lock);
    return ret;
}

/*
free时直接标为dirty
gc时，如果要copy转移某个slab，需要在新chunk里为它分配一个新的sregion
gc时，需要顺次扫描arena中记录的chunk链表，每个chunk顺次扫描sregion，每个sregion顺次扫描slab
*/
//TODO: 加锁。如果需要arena锁，slab要增加指向sregion的索引，sregion要增加指向chunk的索引
void 
arena_dalloc_small(pslab_t * slab)
{
    //malloc_mutex_lock(&arena->lock);
	slab->attr = 'd';
	//malloc_mutex_unlock(&arena->lock);
}
