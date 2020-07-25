#define	LSMALLOC_BASE_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */

static malloc_mutex_t	base_mtx;

/*
 * Current pages that are being used for internal memory allocations.  These
 * pages are carved up in cacheline-size quanta, so that there is no chance of
 * false cache line sharing.
 */
static void		*base_pages;
static void		*base_next_addr;
static void		*base_past_addr; /* Addr immediately past base_pages. */

/******************************************************************************/
/* Function prototypes for non-inline static functions. */

static bool	base_pages_alloc(size_t minsize);

/******************************************************************************/

static bool
base_pages_alloc(size_t minsize)
{
	size_t csize;
	bool zero;

	assert(minsize != 0);
	csize = CHUNK_CEILING(minsize);
	zero = false;
	base_pages = chunk_alloc(csize, chunksize, true, &zero,
	    chunk_dss_prec_get());
	if (base_pages == NULL)
		return (true);
	base_next_addr = base_pages;
	base_past_addr = (void *)((uintptr_t)base_pages + csize);

	return (false);
}

void *
base_alloc(size_t size)
{
	void *ret;
	size_t csize;

	/* Round size up to nearest multiple of the cacheline size. */
	csize = CACHELINE_CEILING(size);

	malloc_mutex_lock(&base_mtx);
	/* Make sure there's enough space for the allocation. */
	if ((uintptr_t)base_next_addr + csize > (uintptr_t)base_past_addr) {
		if (base_pages_alloc(csize)) {
			malloc_mutex_unlock(&base_mtx);
			return (NULL);
		}
	}
	/* Allocate. */
	ret = base_next_addr;
	base_next_addr = (void *)((uintptr_t)base_next_addr + csize);
	malloc_mutex_unlock(&base_mtx);

	return (ret);
}

void *
base_calloc(size_t number, size_t size)
{
	void *ret = base_alloc(number * size);

	if (ret != NULL)
		memset(ret, 0, number * size);

	return (ret);
}

bool
base_boot(void)
{

	if (malloc_mutex_init(&base_mtx))
		return (true);

	return (false);
}

void
base_prefork(void)
{

	malloc_mutex_prefork(&base_mtx);
}

void
base_postfork_parent(void)
{

	malloc_mutex_postfork_parent(&base_mtx);
}

void
base_postfork_child(void)
{

	malloc_mutex_postfork_child(&base_mtx);
}
