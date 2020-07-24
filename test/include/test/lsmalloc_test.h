#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <pthread.h>


/******************************************************************************/
/*
 * Define always-enabled assertion macros, so that test assertions execute even
 * if assertions are disabled in the library code.  These definitions must
 * exist prior to including "jemalloc/internal/util.h".
 */
#define	assert(e) do {							\
	if (!(e)) {							\
		malloc_printf(						\
		    "<jemalloc>: %s:%d: Failed assertion: \"%s\"\n",	\
		    __FILE__, __LINE__, #e);				\
		abort();						\
	}								\
} while (0)

#define	not_reached() do {						\
	malloc_printf(							\
	    "<jemalloc>: %s:%d: Unreachable code reached\n",		\
	    __FILE__, __LINE__);					\
	abort();							\
} while (0)

#define	not_implemented() do {						\
	malloc_printf("<jemalloc>: %s:%d: Not implemented\n",		\
	    __FILE__, __LINE__);					\
	abort();							\
} while (0)

#define	assert_not_implemented(e) do {					\
	if (!(e))							\
		not_implemented();					\
} while (0)

#include "test/lsmalloc_test_defs.h"




/******************************************************************************/
/*
 * For unit tests, expose all public and private interfaces.
 */
#ifdef LSMALLOC_UNIT_TEST

#include "lsmalloc/internal/lsmalloc_internal.h"

#endif
/******************************************************************************/
/*
 * Common test utilities.
 */
#include "test/test.h"