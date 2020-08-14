#ifdef LSMALLOC_H_TYPES

//暂定
#define sregion_size 1024
#define CLASS_NUM 12
const unsigned short size_class[CLASS_NUM]={8, 16, 32, 48, 64, 80, 96, 112, 128, 160, 192, 224}

#endif /* JEMALLOC_H_TYPES */
/******************************************************************************/
#ifdef LSMALLOC_H_STRUCTS

#endif /* LSMALLOC_H_STRUCTS */
/******************************************************************************/
#ifdef LSMALLOC_H_EXTERNS

unsigned short size_to_class(size_t size)
{
    //assert(size <= arena_maxsmall);
    int i;
    for (i = 0; i < CLASS_NUM; i++)
    {
        if (size[i] >= size)
        {
            return i;
        }
    }
}


#endif /* LSMALLOC_H_EXTERNS */
/******************************************************************************/
#ifdef LSMALLOC_H_INLINES

#endif /* LSMALLOC_H_INLINES */
/******************************************************************************/