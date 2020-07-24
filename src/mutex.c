#define	LSMALLOC_MUTEX_C_
#include "lsmalloc/internal/lsmalloc_internal.h"


#ifndef _CRT_SPINCOUNT
#define	_CRT_SPINCOUNT 4000
#endif

/******************************************************************************/
/* Data. */



/******************************************************************************/


bool
malloc_mutex_init(malloc_mutex_t *mutex)
{


	pthread_mutexattr_t attr;

	if (pthread_mutexattr_init(&attr) != 0)
		return (true);
	/* 编译时的-D_GNU_SOURCE会启用PTHREAD_MUTEX_DEFAULT */
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_DEFAULT);
	if (pthread_mutex_init(&mutex->lock, &attr) != 0) {
		pthread_mutexattr_destroy(&attr);
		return (true);
	}
	pthread_mutexattr_destroy(&attr);

	return (false);
}

void
malloc_mutex_prefork(malloc_mutex_t *mutex)
{

	malloc_mutex_lock(mutex);
}

void
malloc_mutex_postfork_parent(malloc_mutex_t *mutex)
{

	malloc_mutex_unlock(mutex);
}

void
malloc_mutex_postfork_child(malloc_mutex_t *mutex)
{

	if (malloc_mutex_init(mutex)) {
		malloc_printf("<lsmalloc>: Error re-initializing mutex in "
		    "child\n");
			abort();
	}
}

bool
mutex_boot(void)
{
	return (false);
}
