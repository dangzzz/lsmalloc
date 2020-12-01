/******************************************************************************/
#ifdef LSMALLOC_H_TYPES

#define PMEMPOOL_SIZE 10737418240

//#define PMEMPOOL_SIZE 160 * 1024 * 1024

typedef struct pmempool_s pmempool_t;
typedef struct freelist_s freelist_t;
typedef struct filelist_s filelist_t;

#endif /* LSMALLOC_H_TYPES */
/******************************************************************************/
#ifdef LSMALLOC_H_STRUCTS

struct freelist_s
{
	qr(freelist_t) link;

	/*pmem地址*/
	void *paddr;
};

struct filelist_s
{
	qr(filelist_t) link;

	/*用于标识文件*/
	unsigned int file_no;

	/*mempool起始地址*/
	void *pool_paddr;
};

struct pmempool_s
{
	/*记录文件的标号及对应pmem起始地址*/
	filelist_t *file;

	/*当前可用于分配的内存块链表*/
	freelist_t *fl_now;

	/*pool中的chunk总数*/
	int chunk_tot;

	/*已分配出去的chunk总数*/
	int chunk_used;
};

#endif /* LSMALLOC_H_STRUCTS */
/******************************************************************************/
#ifdef LSMALLOC_H_EXTERNS
void pmempool_create(pmempool_t *pp);
void pmempool_destroy(pmempool_t *pp);
void *pmempool_chunk_alloc(pmempool_t *pp);
void pmempool_free(pmempool_t *pp, void *ptr);
int pmempool_usedpct(pmempool_t *pp);

#endif /* LSMALLOC_H_EXTERNS */
/******************************************************************************/
#ifdef LSMALLOC_H_INLINES

#endif /* LSMALLOC_H_INLINES */
/******************************************************************************/
