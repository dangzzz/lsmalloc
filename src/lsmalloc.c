#define	LSMALLOC_TEMPLATE_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */


malloc_mutex_t		arenas_lock;
arena_t			**arenas;
unsigned		narenas_total;
unsigned		narenas_auto;

/* Set to true once the allocator has been initialized. */
static bool		malloc_initialized = false;

static malloc_mutex_t	init_lock = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************************/
/* Function prototypes for non-inline static functions. */


/******************************************************************************/
/* Inline tool function */


/******************************************************************************/

static void
malloc_init_hard(void){
    malloc_mutex_lock(&init_lock);
	if (malloc_initialized) {
		/*
		 * Another thread initialized the allocator before this one
		 * acquired init_lock, or this thread is the initializing
		 * thread, and it is recursively allocating.
		 */
		malloc_mutex_unlock(&init_lock);
		return;
	}

    arena_boot();

	malloc_initialized = true;
	malloc_mutex_unlock(&init_lock);
}

void *
lsmalloc(size_t size,void **ptr){
    void *ret;
    if(size == 0)
        size =1;

    if(malloc_initialized == false){
        malloc_init_hard();
    }

    ret = imalloc(size,ptr);

    return ret;
}