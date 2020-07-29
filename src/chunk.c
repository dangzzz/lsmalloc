#define	LSMALLOC_CHUNK_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */


size_t		opt_lg_chunk = LG_CHUNK_DEFAULT;

malloc_mutex_t	chunks_mtx;


/* Various chunk-related settings. */
size_t		chunksize;
size_t		chunksize_mask; /* (chunksize - 1). */
size_t		chunk_npages;

/******************************************************************************/
/* Function prototypes for non-inline static functions. */



/******************************************************************************/

/*
 * If the caller specifies (*zero == false), it is still possible to receive
 * zeroed memory, in which case *zero is toggled to true.  arena_chunk_alloc()
 * takes advantage of this to avoid demanding zeroed chunks, but taking
 * advantage of them if they are returned.
 */
void *
chunk_alloc(size_t size, size_t alignment, bool base, bool *zero)
{
	void *ret;

	assert(size != 0);
	assert((size & chunksize_mask) == 0);
	assert(alignment != 0);
	assert((alignment & chunksize_mask) == 0);


	/* mmap. */

	ret = chunk_alloc_mmap(size, alignment, zero);

	assert(CHUNK_ADDR2BASE(ret) == ret);
	return (ret);
}



void
chunk_unmap(void *chunk, size_t size)
{
	assert(chunk != NULL);
	assert(CHUNK_ADDR2BASE(chunk) == chunk);
	assert(size != 0);
	assert((size & chunksize_mask) == 0);

	chunk_dealloc_mmap(chunk, size);

}

void
chunk_dealloc(void *chunk, size_t size, bool unmap)
{

	assert(chunk != NULL);
	assert(CHUNK_ADDR2BASE(chunk) == chunk);
	assert(size != 0);
	assert((size & chunksize_mask) == 0);

	if (unmap)
		chunk_unmap(chunk, size);
}

bool
chunk_boot(void)
{

	/* Set variables according to the value of opt_lg_chunk. */
	chunksize = (ZU(1) << opt_lg_chunk);
	assert(chunksize >= PAGE);

	chunksize_mask = chunksize - 1;
	chunk_npages = (chunksize >> LG_PAGE);

	return (false);
}

void
chunk_prefork(void)
{

	malloc_mutex_prefork(&chunks_mtx);
}

void
chunk_postfork_parent(void)
{

	malloc_mutex_postfork_parent(&chunks_mtx);
}

void
chunk_postfork_child(void)
{

	malloc_mutex_postfork_child(&chunks_mtx);
}
