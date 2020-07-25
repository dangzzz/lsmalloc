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


#include "lsmalloc/internal/util.h"
#include "lsmalloc/internal/pool.h"
#include "lsmalloc/internal/mutex.h"
#include "lsmalloc/internal/tsd.h"
#include "lsmalloc/internal/arena.h"

#undef LSMALLOC_H_TYPES
/******************************************************************************/
#define	LSMALLOC_H_STRUCTS


#include "lsmalloc/internal/util.h"
#include "lsmalloc/internal/pool.h"
#include "lsmalloc/internal/mutex.h"
#include "lsmalloc/internal/tsd.h"
#include "lsmalloc/internal/arena.h"

#undef LSMALLOC_H_STRUCTS
/******************************************************************************/
#define	LSMALLOC_H_EXTERNS

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

#include "lsmalloc/internal/util.h"
#include "lsmalloc/internal/pool.h"
#include "lsmalloc/internal/mutex.h"
#include "lsmalloc/internal/tsd.h"
#include "lsmalloc/internal/arena.h"

#undef LSMALLOC_H_EXTERNS
/******************************************************************************/
#define	LSMALLOC_H_INLINES


#include "lsmalloc/internal/util.h"
#include "lsmalloc/internal/pool.h"
#include "lsmalloc/internal/mutex.h"
#include "lsmalloc/internal/tsd.h"
#include "lsmalloc/internal/arena.h"

#ifndef LSMALLOC_ENABLE_INLINE
malloc_tsd_protos(LSMALLOC_ATTR(unused), arenas, arena_t *)


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

LSMALLOC_ALWAYS_INLINE void *
ilmalloct(size_t size, arena_t *arena, void **ptr)
{

	assert(size != 0);

	
	return (arena_malloc(arena, size, false, ptr));
}

LSMALLOC_ALWAYS_INLINE void *
imalloc(size_t size,void **ptr){
    return (imalloct(size,NULL,ptr));
}

#undef LSMALLOC_H_INLINES
/******************************************************************************/
#endif /* LSMALLOC_INTERNAL_H */