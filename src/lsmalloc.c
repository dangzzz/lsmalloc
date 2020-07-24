#define	LSMALLOC_TEMPLATE_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */


malloc_mutex_t		arenas_lock;
arena_t			**arenas;
unsigned		narenas_total;
unsigned		narenas_auto;

/******************************************************************************/
/* Function prototypes for non-inline static functions. */


/******************************************************************************/
/* Inline tool function */


/******************************************************************************/