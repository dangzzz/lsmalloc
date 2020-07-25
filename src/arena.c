#define	LSMALLOC_TEMPLATE_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */

size_t      arena_maxlarge;
size_t      arena_maxsmall;

/******************************************************************************/
/* Function prototypes for non-inline static functions. */


/******************************************************************************/
/* Inline tool function */


/******************************************************************************/


void
arena_boot(void)
{
    //todo
    arena_maxlarge = 0;
    arena_maxsmall = 0;
}