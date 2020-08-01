#define	LSMALLOC_HUGE_C_
#include "lsmalloc/internal/lsmalloc_internal.h"

/******************************************************************************/
/* Data. */
extern int mmap_file;
extern int pmem_consmp;

static huge_tree_t huge_tree;
/******************************************************************************/
/* Function prototypes for non-inline static functions. */


/******************************************************************************/
/* Inline tool function */


/******************************************************************************/
static inline int
huge_ad_cmp(huge_s *a , huge_s *b)
{
	int ret;
	uintptr_t a_addr = (uintptr_t)a->addr;
	uintptr_t b_addr = (uintptr_t)b->addr;
	ret = (a_addr > b_addr) - (a_addr < b_addr);
	return ret;
}

/*Generate red-black tree functions*/
rb_gen(, huge_tree_, huge_tree_t, huge_t, link_ad, huge_ad_cmp)

//TODO:锁
void huge_alloc(size_t size)
{
	void * ret, *addr;
	char str[32];
	size_t mapped_len;
	int is_pmem;
	huge_t node; //暂时先这样用

	/*mmap*/
	sprintf(str,"/mnt/pmem/%d",mmap_file);
	mmap_file++;  //lock

	if((addr=pmem_map_file(str,size,PMEM_FILE_CREATE,0666,&mapped_len, &is_pmem))==NULL){
		perror("pmem_map_file");
		exit(1);
	}

	pmem_consmp += size; 
	ret = addr;

	/*初始化结点，并插入树中*/
	node.file_no = mmap_file;
	node.addr = ret;
	node.size = size;
 	
	//lock
	huge_tree_insert(&huge_tree, &node);	
}

//TODO:锁
void huge_dalloc(void *ptr)
{
	huge_t *node, key;
	char str[32];

	key.addr = ptr;
	//lock
	node = huge_tree_search(&huge_tree, &key);
	assert(node != NULL);
	assert(node->addr == ptr);
	huge_tree_remove(&huge, node);
	
	sprintf(str,"/mnt/pmem/%d",node->file_no);
	pmem_unmap(node->addr, node->size);
	pmem_consmp -= node->size;
	remove(str);
}
