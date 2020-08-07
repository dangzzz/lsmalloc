/******************************************************************************/
#ifdef LSMALLOC_H_TYPES

#define REGION_ALIVE (unsigned short)0x1U;
#define REGION_DIRTY (unsigned short)0x2U;
#define CHUNK_TYPE_SLAB	's'
#define CHUNK_TYPE_LOG	'l'

typedef struct arena_s arena_t;
typedef struct chunk_s chunk_t;
typedef struct region_s region_t;
typedef struct pregion_s pregion_t;
typedef struct pchunk_s pchunk_t;
#endif /* LSMALLOC_H_TYPES */
/******************************************************************************/
#ifdef LSMALLOC_H_STRUCTS


struct pregion_s{
	region_t	*region;
};

struct pchunk_s{
	chunk_t		*chunk;
};

struct region_s{
	ql_elm(region_t)	regions_link;

	unsigned short		attr;
	unsigned short		threadid;
	size_t				size;
	void **				ptr;
	void*				paddr;
	chunk_t				*chunk;
};

struct chunk_s{

	ql_head(region_t) 	regions;

	arena_t					*arena;

	void					*tail;

	void					*paddr;

	ql_elm(chunk_t)	avail_link;

	size_t				availsize;

	char				chunktype;
};



struct arena_s {


	/* This arena's index within the arenas array. */
	unsigned		ind;

	/*
	 * Number of threads currently assigned to this arena.  This field is
	 * protected by arenas_lock.
	 */
	unsigned		nthreads;

	/*
	 * There are three classes of arena operations from a locking
	 * perspective:
	 * 1) Thread asssignment (modifies nthreads) is protected by
	 *    arenas_lock.
	 * 2) Bin-related operations are protected by bin locks.
	 * 3) Chunk- and run-related operations are protected by this mutex.
	 */
	malloc_mutex_t		lock;

	pmempool_t				pool;

	ql_head(chunk_t) 	avail_chunks;

	chunk_t				*maxchunk;
};

#endif /* LSMALLOC_H_STRUCTS */
/******************************************************************************/
#ifdef LSMALLOC_H_EXTERNS

extern size_t arena_maxlarge;
extern size_t arena_maxsmall;
void	arena_boot(void);
bool	arena_new(arena_t *arena, unsigned ind);
void	*arena_malloc_large(arena_t *arena,size_t size, bool zero, void **ptr);
void	arena_dalloc_large(arena_t *arena,chunk_t *chunk,region_t *region);
#endif /* LSMALLOC_H_EXTERNS */
/******************************************************************************/
#ifdef LSMALLOC_H_INLINES

LSMALLOC_ALWAYS_INLINE void *
arena_malloc(arena_t *arena, size_t size, bool zero, void **ptr)
{

	assert(size != 0);

	if(size <= arena_maxsmall){
		return NULL;
		//return(arena_malloc_small(choose_arena(arena), size, zero, ptr));
	}else{
		return(arena_malloc_large(choose_arena(arena), size, zero, ptr));
	}

}

LSMALLOC_ALWAYS_INLINE void 
arena_dalloc(chunk_t *chunk,void* ptr)
{
	if(chunk->chunktype == CHUNK_TYPE_LOG){
		region_t *region;
		region = ((pregion_t *)((intptr_t)ptr-sizeof(pregion_t)))->region;
		arena_dalloc_large(chunk->arena,chunk,region);
	}else if(chunk->chunktype == CHUNK_TYPE_SLAB){

	}

}
#endif /* LSMALLOC_H_INLINES */
/******************************************************************************/