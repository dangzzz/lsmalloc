/******************************************************************************/
#ifdef LSMALLOC_H_TYPES

typedef struct gc_info_s gc_info_t;
typedef struct fake_chunk_s fake_chunk_t;
typedef struct slowgc_task_s slowgc_task_t;
#endif /* LSMALLOC_H_TYPES */
/******************************************************************************/
#ifdef LSMALLOC_H_STRUCTS

struct gc_info_s{   
    arena_t *arena;
    fake_chunk_t *fake;
};

struct fake_chunk_s{
    char				chunktype; 
    sl_elm(chunk_t)    avail_link;
};

struct slowgc_task_s{
    arena_t         *arena;
    chunk_t         *start;
    chunk_t         *first;
}

#endif /* LSMALLOC_H_STRUCTS */
/******************************************************************************/
#ifdef LSMALLOC_H_EXTERNS

void maybe_gc(arena_t *arena);

#endif /* LSMALLOC_H_EXTERNS */
/******************************************************************************/
#ifdef LSMALLOC_H_INLINES

#endif /* LSMALLOC_H_INLINES */
/******************************************************************************/

