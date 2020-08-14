#define	LSMALLOC_SLAB_C_
#include "lsmalloc/internal/lsmalloc_internal.h"
/******************************************************************************/
/* Data. */
const size_t size_class[CLASS_NUM]={8, 16, 32, 48, 64, 80, 96, 112, 128, 160, 192, 224};

/******************************************************************************/
/* Function prototypes for non-inline static functions. */


/******************************************************************************/
/* Inline tool function */


/******************************************************************************/
//TODO: 改成更快的查找方式
unsigned short size_to_class(size_t size)
{
    //assert(size <= arena_maxsmall);
    int i, ret;
    for (i = 0; i < CLASS_NUM; i++)
    {
        if (size_class[i] >= size)
        {
            ret = i;
            break;
        }
    }
    return ret;
}

size_t class_to_size(unsigned short cls)
{
    return size_class[cls];
}