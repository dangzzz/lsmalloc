#define	LSMALLOC_TEMPLATE_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */

malloc_tsd_data(, arenas, arena_t *, NULL)
//lsmalloc.h   malloc_tsd_protos malloc_tsd_funcs
malloc_tsd_data(, lid, unsigned short, NULL)

size_t	opt_narenas = 0;

unsigned	ncpus;

malloc_mutex_t		arenas_lock;
arena_t			**arenas;
unsigned		narenas_total;
unsigned		narenas_auto;

/*用于更新lid*/
static unsigned short lthread_cnt = 0;
/*每个线程拥有不同的lid，用于标识线程*/
//__thread unsigned short lid = 0;  

/* Set to true once the allocator has been initialized. */
static bool		malloc_initialized = false;

static malloc_mutex_t	init_lock = PTHREAD_MUTEX_INITIALIZER;

char * pmem_path;
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

	if (narenas_auto > 1) {
		unsigned i, choose, first_null;

		choose = 0;
		first_null = narenas_auto;
		malloc_mutex_lock(&arenas_lock);
		assert(arenas[0] != NULL);
		for (i = 1; i < narenas_auto; i++) {
			if (arenas[i] != NULL) {
				/*
				 * Choose the first arena that has the lowest
				 * number of threads assigned to it.
				 */
				if (arenas[i]->nthreads <
				    arenas[choose]->nthreads)
					choose = i;
			} else if (first_null == narenas_auto) {
				/*
				 * Record the index of the first uninitialized
				 * arena, in case all extant arenas are in use.
				 *
				 * NB: It is possible for there to be
				 * discontinuities in terms of initialized
				 * versus uninitialized arenas, due to the
				 * "thread.arena" mallctl.
				 */
				first_null = i;
			}
		}

		if (arenas[choose]->nthreads == 0
		    || first_null == narenas_auto) {
			/*
			 * Use an unloaded arena, or the least loaded arena if
			 * all arenas are already initialized.
			 */
			ret = arenas[choose];
		} else {
			/* Initialize a new arena. */
			ret = arenas_extend(first_null);
		}
		ret->nthreads++;
		malloc_mutex_unlock(&arenas_lock);
	} else {
		ret = arenas[0];
		malloc_mutex_lock(&arenas_lock);
		ret->nthreads++;
		malloc_mutex_unlock(&arenas_lock);
	}

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
	if (lid_tsd_boot()) {
		assert(false);
	}
	lid_tsd_set(&lthread_cnt);
}

static void
malloc_init_hard(void){
	arena_t *init_arenas[1];

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

	if(chunk_boot()){
		assert(false);
	}

    arena_boot();

	if (malloc_mutex_init(&arenas_lock)) {
		assert(false);
	}



	if (base_boot()) {
		assert(false);
	}




	narenas_total = narenas_auto = 1;
	arenas = init_arenas;
	memset(arenas, 0, sizeof(arena_t *) * narenas_auto);
	arenas_extend(0);

	if (arenas[0] == NULL) {
		malloc_mutex_unlock(&init_lock);
		assert(false);
	}

	if (arenas_tsd_boot()) {
		assert(false);
	}


	ncpus = malloc_ncpus();

	if (opt_narenas == 0) {
		/*
		 * For SMP systems, create more than one arena per CPU by
		 * default.
		 */
		if (ncpus > 1)
			opt_narenas = ncpus << 2;
		else
			opt_narenas = 1;
	}
	narenas_auto = opt_narenas;
	narenas_total = narenas_auto;
	arenas = (arena_t **)malloc(sizeof(arena_t *) * narenas_total);
	if (arenas == NULL) {
		assert(false);
	}
	

	memset(arenas, 0, sizeof(arena_t *) * narenas_total);

	/* Copy the pointer to the one arena that was already initialized. */
	arenas[0] = init_arenas[0];


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

static void idir(const char *path)
{
	pmem_path = path;
}

void
lspmemdir(const char *path)
{
	idir(path);
}
/*
 * End malloc(3)-compatible functions.
 */
/******************************************************************************/
