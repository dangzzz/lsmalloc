//todo not any more using base,change to slab
/******************************************************************************/
#ifdef LSMALLOC_H_TYPES

#endif /* LSMALLOC_H_TYPES */
/******************************************************************************/
#ifdef LSMALLOC_H_STRUCTS

#endif /* LSMALLOC_H_STRUCTS */
/******************************************************************************/
#ifdef LSMALLOC_H_EXTERNS

void	*base_alloc(size_t size);
void	*base_calloc(size_t number, size_t size);
bool	base_boot(void);
void	base_prefork(void);
void	base_postfork_parent(void);
void	base_postfork_child(void);

#endif /* LSMALLOC_H_EXTERNS */
/******************************************************************************/
#ifdef LSMALLOC_H_INLINES

#endif /* LSMALLOC_H_INLINES */
/******************************************************************************/
