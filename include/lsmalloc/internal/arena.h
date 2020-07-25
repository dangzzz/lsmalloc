/******************************************************************************/
#ifdef LSMALLOC_H_TYPES

typedef struct arena_s arena_t;

#endif /* LSMALLOC_H_TYPES */
/******************************************************************************/
#ifdef LSMALLOC_H_STRUCTS


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

};

#endif /* LSMALLOC_H_STRUCTS */
/******************************************************************************/
#ifdef LSMALLOC_H_EXTERNS

extern size_t arena_maxlarge;
extern size_t arena_maxsmall;
void	arena_boot(void);

#endif /* LSMALLOC_H_EXTERNS */
/******************************************************************************/
#ifdef LSMALLOC_H_INLINES

LSMALLOC_ALWAYS_INLINE void *
arena_malloc(arena_t *arena, size_t size, bool zero, void **ptr)
{

	assert(size != 0);

	if(size <= arena_maxsmall){
		return(arena_malloc_small(choose_arena(arena), size, zero, ptr));
	}else if(size <= arena_maxlarge){
		return(arena_malloc_large(choose_arena(arena), size, zero, ptr));
	}else{
		return(arena_malloc_huge(choose_arena(arena), size, zero));
	}

}

#endif /* LSMALLOC_H_INLINES */
/******************************************************************************/