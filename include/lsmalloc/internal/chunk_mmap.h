/******************************************************************************/
#ifdef LSMALLOC_H_TYPES

#endif /* LSMALLOC_H_TYPES */
/******************************************************************************/
#ifdef LSMALLOC_H_STRUCTS

#endif /* LSMALLOC_H_STRUCTS */
/******************************************************************************/
#ifdef LSMALLOC_H_EXTERNS

bool	pages_purge(void *addr, size_t length);

void	*chunk_alloc_mmap(size_t size, size_t alignment, bool *zero);
bool	chunk_dealloc_mmap(void *chunk, size_t size);

#endif /* LSMALLOC_H_EXTERNS */
/******************************************************************************/
#ifdef LSMALLOC_H_INLINES

#endif /* LSMALLOC_H_INLINES */
/******************************************************************************/
