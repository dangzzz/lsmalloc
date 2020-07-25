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