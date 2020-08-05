#define	LSMALLOC_HUGE_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */
extern unsigned int mmap_file;
extern unsigned int pmem_consmp;

malloc_mutex_t huge_mtx;
static huge_tree_t huge_tree;
/******************************************************************************/
/* Function prototypes for non-inline static functions. */


/******************************************************************************/
/* Inline tool function */


/******************************************************************************/
static inline int
huge_ad_cmp(huge_t *a , huge_t *b)
{
	int ret;
	uintptr_t a_addr = (uintptr_t)a->paddr;
	uintptr_t b_addr = (uintptr_t)b->paddr;
	ret = (a_addr > b_addr) - (a_addr < b_addr);
	return ret;
}

/*Generate red-black tree functions*/
rb_gen(, huge_tree_, huge_tree_t, huge_t, link_ad, huge_ad_cmp)

bool huge_boot()
{
	if (malloc_mutex_init(&huge_mtx))
	{
		return true;
	}
	huge_tree_new(&huge_tree);
	return false;
}


void * huge_alloc(size_t size)
{
	void * ret, *addr;
	char str[32];
	size_t mapped_len;
	int is_pmem;
	huge_t node; //暂时先这样用

	/*mmap，和chunksize对齐*/
	sprintf(str,"/mnt/pmem/%d",mmap_file);
	atomic_add_uint32(&mmap_file, 1);//mmap_file++;

	if((addr=pmem_map_file(str,size,PMEM_FILE_CREATE,0666,&mapped_len, &is_pmem))==NULL)
	{
		perror("pmem_map_file");
		exit(1);
	}

	if((uintptr_t)addr==ALIGNMENT_CEILING((uintptr_t)addr, chunksize))
	{
		ret = addr;
	}
	else
	{
		pmem_unmap(addr,size);
		remove(str);
		if((addr=pmem_map_file(str,2*size,PMEM_FILE_CREATE,0666,&mapped_len, &is_pmem))==NULL)
		{
			perror("pmem_map_file");
			exit(1);
		}
		ret = (void*)ALIGNMENT_CEILING((uintptr_t)addr, chunksize);
		pmem_unmap(addr,((intptr_t)ret-(intptr_t)addr));
		pmem_unmap((void*)((intptr_t)ret+size),((intptr_t)addr+size-(intptr_t)ret));
	}

	atomic_add_uint32(&pmem_consmp, size);//pmem_consmp += size; 

	/*初始化结点，并插入树中*/
	node.file_no = mmap_file;
	node.paddr = ret;
	node.size = size;
 	
	malloc_mutex_lock(&huge_mtx);
	huge_tree_insert(&huge_tree, &node);	
	malloc_mutex_unlock(&huge_mtx);

	return ret;
}

void huge_dalloc(void *ptr)
{
	huge_t *node, key;
	char str[32];

	key.paddr = ptr;
	
	malloc_mutex_lock(&huge_mtx);
	node = huge_tree_search(&huge_tree, &key);
	assert(node != NULL);
	assert(node->paddr == ptr);
	huge_tree_remove(&huge_tree, node);
	malloc_mutex_unlock(&huge_mtx);
	
	sprintf(str,"/mnt/pmem/%d",node->file_no);
	pmem_unmap(node->paddr, node->size);
	atomic_sub_uint32(&pmem_consmp, node->size);//pmem_consmp -= node->size;
	remove(str);
}
