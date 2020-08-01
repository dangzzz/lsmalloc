#define	LSMALLOC_TEMPLATE_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */

malloc_tsd_data(, arenas, arena_t *, NULL)
//lsmalloc.h   malloc_tsd_protos malloc_tsd_funcs
malloc_tsd_data(, lid, unsigned short, NULL)

unsigned	ncpus;
//todo 动态分配和找最少,目前一个thread一个
malloc_mutex_t		arenas_lock;
arena_t			*arenas[50];

/*用于更新lid*/
static unsigned short lthread_cnt = 0;
/*每个线程拥有不同的lid，用于标识线程*/
//__thread unsigned short lid = 0;  

/* Set to true once the allocator has been initialized. */
static bool		malloc_initialized = false;

static malloc_mutex_t	init_lock = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************************/
/* Function prototypes for non-inline static functions. */


/******************************************************************************/
/*
 * Begin miscellaneous support functions.
 */

/* Create a new arena and insert it into the arenas array at index ind. */
arena_t *
arenas_extend(unsigned ind)
{
	arena_t *ret;

	ret = (arena_t *)malloc(sizeof(arena_t));
	if (ret != NULL && arena_new(ret, ind) == false) {
		arenas[ind] = ret;
		return (ret);
	}
	/* Only reached if there is an OOM error. */

	/*
	 * OOM here is quite inconvenient to propagate, since dealing with it
	 * would require a check for failure in the fast path.  Instead, punt
	 * by using arenas[0].  In practice, this is an extremely unlikely
	 * failure.
	 */
	malloc_write("<jemalloc>: Error initializing arena\n");
	abort();

	return (arenas[0]);
}


/* Slow path, called only by choose_arena(). */
arena_t *
choose_arena_hard(void)
{
	arena_t *ret;

	unsigned i, choose;

	choose = 0;

	malloc_mutex_lock(&arenas_lock);
	assert(arenas[0] != NULL);
	for (i = 0; i < sizeof(arenas)/sizeof(arena_t *); i++) {
		if (arenas[i] != NULL) {
				choose = i;
		}
	}


	ret = arenas_extend(choose);

	ret->nthreads++;
	malloc_mutex_unlock(&arenas_lock);

	arenas_tsd_set(&ret);

	return (ret);
}


/*
 * End miscellaneous support functions.
 */
/******************************************************************************/
/*
 * Begin initialization functions.
 */

void
arenas_cleanup(void *arg)
{
	arena_t *arena = *(arena_t **)arg;

	malloc_mutex_lock(&arenas_lock);
	arena->nthreads--;
	malloc_mutex_unlock(&arenas_lock);
}


static unsigned
malloc_ncpus(void)
{
	long result;

	result = sysconf(_SC_NPROCESSORS_ONLN);

	return ((result == -1) ? 1 : (unsigned)result);
}

//todo __thread to tsd
static void inline
lid_boot(){
	lid_tsd_set(&lthread_cnt);
}

static void
malloc_init_hard(void){
    malloc_mutex_lock(&init_lock);
	if (malloc_initialized) {
		/*
		 * Another thread initialized the allocator before this one
		 * acquired init_lock, or this thread is the initializing
		 * thread, and it is recursively allocating.
		 */
		malloc_mutex_unlock(&init_lock);
		return;
	}

	lid_boot();

    arena_boot();

	if (malloc_mutex_init(&arenas_lock)) {
		assert(false);
	}

	if (base_boot()) {
		assert(false);
	}

	if (arenas_tsd_boot()) {
		assert(false);
	}

	ncpus = malloc_ncpus();


	malloc_initialized = true;
	malloc_mutex_unlock(&init_lock);
}
/*
 * End initialization functions.
 */
/******************************************************************************/
/*
 * Begin malloc(3)-compatible functions.
 */
void *
lsmalloc(size_t size,void **ptr){
    void *ret;
    if(size == 0)
        size =1;

    if(malloc_initialized == false){
        malloc_init_hard();
    }

    ret = imalloc(size,ptr);

    return ret;
}



static inline void
ifree(void *ptr){
	idalloc(ptr);
}

void
lsfree(void *ptr)
{

	if (ptr != NULL)
		ifree(ptr);
}

/*
 * End malloc(3)-compatible functions.
 */
/******************************************************************************/
