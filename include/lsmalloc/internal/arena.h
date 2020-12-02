/******************************************************************************/
#ifdef LSMALLOC_H_TYPES

#define REGION_ALIVE (unsigned short)0x1U
#define REGION_DIRTY (unsigned short)0x2U
#define CHUNK_TYPE_SLAB	's'
#define CHUNK_TYPE_LOG	'l'
#define CHUNK_TYPE_GC	'g'
#define CHUNK_TYPE_FAKE	'f'
#define SLAB_DIRTY 'd'
#define SLAB_CLEAN 'c'

typedef struct arena_s arena_t;
typedef struct chunk_s chunk_t;
typedef struct region_s region_t;
typedef struct pregion_s pregion_t;
typedef struct pchunk_s pchunk_t;
typedef struct sregion_s sregion_t;
typedef struct psregion_s psregion_t;
typedef struct pslab_s pslab_t;

#endif /* LSMALLOC_H_TYPES */
/******************************************************************************/
#ifdef LSMALLOC_H_STRUCTS


struct pregion_s{
	region_t	*region;
};

/*schunk和lchunk都使用这个*/
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

/*两个类型的chunk共用*/
struct chunk_s{
	//第一个必须是chunktype
	char				chunktype;   //
	
	sl_elm(chunk_t)	avail_link;	 //

	ql_head(region_t) 	regions;

	arena_t					*arena;   //

	void					*tail;    //

	void					*paddr;    //


	size_t				availsize;    //
	size_t				dirtysize;


	malloc_mutex_t 		gc_lock;
	malloc_mutex_t 		write_lock;

	bool 				live;

	bool				gcing;

};

/*以下是small数据的数据结构*/
/*暂时并不需要将sregion关联到chunk*/
struct sregion_s
{
	/*内部slab的size*/
	size_t size;

	/*pmem中数据区域的起始地址*/
	void * paddr; 

	/*现在分配到的位置尾指针*/
	void * ptail; 
};

/*pmem中的sregion头部，存放一个指向dram中元数据的指针*/
struct psregion_s
{
	sregion_t * sregion;
};

/*slab的元数据，暂定和pmem中用户数据放在一起，不然指针空间开销太大了*/
/*总大小：9B*/
/*暂时并不需要将slab关联到region*/
struct pslab_s
{
	/*用户提供的*/
	void ** ptr;

	/*记录dirty or not*/
	char attr;
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


	sl_head(chunk_t) 	avail_chunks;
	
	/*已分配的schunk的链表插入到这个链表中*/
	sl_head(chunk_t)  avail_schunks;

	chunk_t				*maxchunk;

	/*对于某个size class，当前可用来分配的sregion*/
	//TODO class_num
	sregion_t *  avail_sregion[CLASS_NUM];

	chunk_t				*gc_chunk;

	malloc_mutex_t		gc_lock;
	
	unsigned int thread_count;
	sem_t				gc_sem;
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
void	*arena_malloc_small(arena_t *arena,size_t size, bool zero, void **ptr);
void	arena_dalloc_small(pslab_t * slab);
#endif /* LSMALLOC_H_EXTERNS */
/******************************************************************************/
#ifdef LSMALLOC_H_INLINES

LSMALLOC_ALWAYS_INLINE void *
arena_malloc(arena_t *arena, size_t size, bool zero, void **ptr)
{

	assert(size != 0);

	if(size < arena_maxsmall){
		//return NULL;
		
		return(arena_malloc_small(choose_arena(arena), size, zero, ptr));
		
	}else{
		
		return(arena_malloc_large(choose_arena(arena), size, zero, ptr));
		
	}

}

LSMALLOC_ALWAYS_INLINE void 
arena_dalloc(chunk_t *chunk,void* ptr)
{
	if(chunk->chunktype == CHUNK_TYPE_LOG)
	{
		region_t *region;
		region = ((pregion_t *)((intptr_t)ptr-sizeof(pregion_t)))->region;
		arena_dalloc_large(chunk->arena,chunk,region);
	}else if(chunk->chunktype == CHUNK_TYPE_SLAB)
	{
		pslab_t * slab;
		slab = (pslab_t *)((intptr_t)ptr-sizeof(pslab_t));
		arena_dalloc_small(slab);
	}

}
#endif /* LSMALLOC_H_INLINES */
/******************************************************************************/