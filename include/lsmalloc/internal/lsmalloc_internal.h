#ifndef LSMALLOC_INTERNAL_H
#define	LSMALLOC_INTERNAL_H

//在编译时以-D flag的形式定义
//#define _GNU_SOURCE

#  include <sys/param.h>
#  include <sys/mman.h>
#  include <sys/syscall.h>
#  if !defined(SYS_write) && defined(__NR_write)
#    define SYS_write __NR_write
#  endif
#include <sys/uio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "libpmem.h"
#include <unistd.h>
#include "lsmalloc_internal_defs.h"

#include "../lsmalloc.h"


#define RB_COMPACT
#include "lsmalloc/internal/rb.h"
#include "lsmalloc/internal/qr.h"
#include "lsmalloc/internal/ql.h"
/*
 *   LSMALLOC_H_TYPES   : Preprocessor-defined constants and psuedo-opaque data
 *                        types.
 *   LSMALLOC_H_STRUCTS : Data structures.
 *   LSMALLOC_H_EXTERNS : Extern data declarations and function prototypes.
 *   LSMALLOC_H_INLINES : Inline functions.
 */
/******************************************************************************/
#define	LSMALLOC_H_TYPES
#include "lsmalloc/internal/lsmalloc_internal_macros.h"

/*
 * Maximum size of L1 cache line.  This is used to avoid cache line aliasing.
 * In addition, this controls the spacing of cacheline-spaced size classes.
 *
 * CACHELINE cannot be based on LG_CACHELINE because __declspec(align()) can
 * only handle raw constants.
 */
#define	LG_CACHELINE		6
#define	CACHELINE		64
#define	CACHELINE_MASK		(CACHELINE - 1)

/* Return the smallest cacheline multiple that is >= s. */
#define	CACHELINE_CEILING(s)						\
	(((s) + CACHELINE_MASK) & ~CACHELINE_MASK)

/* Page size.  STATIC_PAGE_SHIFT is determined by the configure script. */
#ifdef PAGE_MASK
#  undef PAGE_MASK
#endif
#define	LG_PAGE		STATIC_PAGE_SHIFT
#define	PAGE		((size_t)(1U << STATIC_PAGE_SHIFT))
#define	PAGE_MASK	((size_t)(PAGE - 1))

/* Return the smallest pagesize multiple that is >= s. */
#define	PAGE_CEILING(s)							\
	(((s) + PAGE_MASK) & ~PAGE_MASK)

/* Return the nearest aligned address at or below a. */
#define	ALIGNMENT_ADDR2BASE(a, alignment)				\
	((void *)((uintptr_t)(a) & (-(alignment))))

/* Return the offset between a and the nearest aligned address at or below a. */
#define	ALIGNMENT_ADDR2OFFSET(a, alignment)				\
	((size_t)((uintptr_t)(a) & (alignment - 1)))

/* Return the smallest alignment multiple that is >= s. */
#define	ALIGNMENT_CEILING(s, alignment)					\
	(((s) + (alignment - 1)) & (-(alignment)))


#include "lsmalloc/internal/atomic.h"
#include "lsmalloc/internal/util.h"
#include "lsmalloc/internal/pool.h"
#include "lsmalloc/internal/mutex.h"
#include "lsmalloc/internal/tsd.h"
#include "lsmalloc/internal/huge.h"
#include "lsmalloc/internal/arena.h"
#include "lsmalloc/internal/chunk.h"
#include "lsmalloc/internal/base.h"


#undef LSMALLOC_H_TYPES
/******************************************************************************/
#define	LSMALLOC_H_STRUCTS

#include "lsmalloc/internal/util.h"
#include "lsmalloc/internal/pool.h"
#include "lsmalloc/internal/mutex.h"
#include "lsmalloc/internal/tsd.h"
#include "lsmalloc/internal/huge.h"
#include "lsmalloc/internal/arena.h"
#include "lsmalloc/internal/chunk.h"
#include "lsmalloc/internal/base.h"

#undef LSMALLOC_H_STRUCTS
/******************************************************************************/
#define	LSMALLOC_H_EXTERNS

//extern __thread unsigned short lid;  

/* Number of CPUs. */
extern unsigned		ncpus;

/* Protects arenas initialization (arenas, arenas_total). */
extern malloc_mutex_t	arenas_lock;

/*
 * Arenas that are used to service external requests.  Not all elements of the
 * arenas array are necessarily used; arenas are created lazily as needed.
 */
extern arena_t		**arenas;

arena_t	*arenas_extend(unsigned ind);
void	arenas_cleanup(void *arg);
arena_t	*choose_arena_hard(void);

extern char * pmem_path;

#include "lsmalloc/internal/atomic.h"
#include "lsmalloc/internal/util.h"
#include "lsmalloc/internal/pool.h"
#include "lsmalloc/internal/mutex.h"
#include "lsmalloc/internal/tsd.h"
#include "lsmalloc/internal/huge.h"
#include "lsmalloc/internal/arena.h"
#include "lsmalloc/internal/chunk.h"
#include "lsmalloc/internal/base.h"

#undef LSMALLOC_H_EXTERNS
/******************************************************************************/
#define	LSMALLOC_H_INLINES

#include "lsmalloc/internal/atomic.h"
#include "lsmalloc/internal/util.h"
#include "lsmalloc/internal/pool.h"
#include "lsmalloc/internal/mutex.h"
#include "lsmalloc/internal/tsd.h"
#include "lsmalloc/internal/huge.h"
#include "lsmalloc/internal/chunk.h"
#include "lsmalloc/internal/base.h"

#ifndef LSMALLOC_ENABLE_INLINE
malloc_tsd_protos(LSMALLOC_ATTR(unused), arenas, arena_t *)
malloc_tsd_protos(LSMALLOC_ATTR(unused), lid, unsigned short)

arena_t	*choose_arena(arena_t *arena);
#endif



#if (defined(LSMALLOC_ENABLE_INLINE) || defined(LSMALLOC_C_))

/*
 * Map of pthread_self() --> arenas[???], used for selecting an arena to use
 * for allocations.
 */
malloc_tsd_externs(arenas, arena_t *)
malloc_tsd_funcs(LSMALLOC_ALWAYS_INLINE, arenas, arena_t *, NULL,
    arenas_cleanup)

malloc_tsd_externs(lid, unsigned short)
malloc_tsd_funcs(LSMALLOC_ALWAYS_INLINE, lid, unsigned short, NULL, NULL)


/* Choose an arena based on a per-thread value. */
LSMALLOC_INLINE arena_t *
choose_arena(arena_t *arena)
{
	arena_t *ret;

	if (arena != NULL)
		return (arena);

	if ((ret = *arenas_tsd_get()) == NULL) {
		ret = choose_arena_hard();
		assert(ret != NULL);
	}

	return (ret);
}
#endif

#include "lsmalloc/internal/arena.h"

LSMALLOC_ALWAYS_INLINE void *
imalloct(size_t size, arena_t *arena, void **ptr)
{

	assert(size != 0);
	if(size<=arena_maxlarge)
		return (arena_malloc(arena, size, false, ptr));
	else
		return (huge_malloc(size));
		;
}

LSMALLOC_ALWAYS_INLINE void *
imalloc(size_t size,void **ptr){
    return (imalloct(size,NULL,ptr));
}



LSMALLOC_ALWAYS_INLINE void
idalloct(void *ptr)
{

	assert(ptr != NULL);
	chunk_t *chunk = (chunk_t *)CHUNK_ADDR2BASE(ptr);

	if(chunk!=ptr)
		arena_dalloc(chunk,ptr);
	else
		huge_dalloc(ptr);
		;
}


LSMALLOC_ALWAYS_INLINE void
idalloc(void *ptr)
{

	idalloct(ptr);
}

LSMALLOC_ALWAYS_INLINE void
idir(char *path)
{
	pmem_path = path;	
}

#undef LSMALLOC_H_INLINES
/******************************************************************************/
#endif /* LSMALLOC_INTERNAL_H */
