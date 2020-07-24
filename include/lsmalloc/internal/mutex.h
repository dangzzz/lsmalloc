/******************************************************************************/
#ifdef LSMALLOC_H_TYPES

typedef struct malloc_mutex_s malloc_mutex_t;



#endif /* LSMALLOC_H_TYPES */
/******************************************************************************/
#ifdef LSMALLOC_H_STRUCTS

struct malloc_mutex_s {

	pthread_mutex_t		lock;

};

#endif /* LSMALLOC_H_STRUCTS */
/******************************************************************************/
#ifdef LSMALLOC_H_EXTERNS

bool	malloc_mutex_init(malloc_mutex_t *mutex);
void	malloc_mutex_prefork(malloc_mutex_t *mutex);
void	malloc_mutex_postfork_parent(malloc_mutex_t *mutex);
void	malloc_mutex_postfork_child(malloc_mutex_t *mutex);
bool	mutex_boot(void);

#endif /* LSMALLOC_H_EXTERNS */
/******************************************************************************/
#ifdef LSMALLOC_H_INLINES

#ifndef LSMALLOC_ENABLE_INLINE
void	malloc_mutex_lock(malloc_mutex_t *mutex);
void	malloc_mutex_unlock(malloc_mutex_t *mutex);
#endif

#if (defined(LSMALLOC_ENABLE_INLINE) || defined(LSMALLOC_MUTEX_C_))
LSMALLOC_INLINE void
malloc_mutex_lock(malloc_mutex_t *mutex)
{
	pthread_mutex_lock(&mutex->lock);
}

LSMALLOC_INLINE void
malloc_mutex_unlock(malloc_mutex_t *mutex)
{
	pthread_mutex_unlock(&mutex->lock);
}
#endif

#endif /* LSMALLOC_H_INLINES */
/******************************************************************************/
