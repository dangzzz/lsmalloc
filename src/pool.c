#define	LSMALLOC_POOL_C_
#include "lsmalloc/internal/lsmalloc_internal.h"
//todo per arena has its own pool
/******************************************************************************/
/* Data. */
unsigned int mmap_file = 0; 
unsigned int pmem_consmp = 0;
char * pmem_path;

/******************************************************************************/
/*
 * Function prototypes for static functions that are referenced prior to
 * definition.
 */



/******************************************************************************/
/* Inline tool function */


/******************************************************************************/

/*mmap一个memory pool*/
void pmempool_create_one(pmempool_t * pp)
{
	void * ret, *addr;
	char str[32];
	size_t mapped_len;
	int is_pmem;

	size_t size = PMEMPOOL_SIZE;
	size_t alignment = chunksize;  

	//sprintf(str,"%s/%d", pmem_path, ++mmap_file);
	sprintf(str,"/mnt/pmem/%d", ++mmap_file);

	if((addr=pmem_map_file(str,size,PMEM_FILE_CREATE,0666,&mapped_len, &is_pmem))==NULL){
		perror("pmem_map_file");
		exit(1);
	}

	//alignment:chunksize
	if((uintptr_t)addr==ALIGNMENT_CEILING((uintptr_t)addr, alignment)){
		ret = addr;
	}
	else{
		pmem_unmap(addr,size);
		remove(str);
		if((addr=pmem_map_file(str,2*size,PMEM_FILE_CREATE,0666,&mapped_len, &is_pmem))==NULL){
			perror("pmem_map_file");
			exit(1);
		}
		ret = (void*)ALIGNMENT_CEILING((uintptr_t)addr,alignment);
		pmem_unmap(addr,((intptr_t)ret-(intptr_t)addr));
		pmem_unmap((void*)((intptr_t)ret+size),((intptr_t)addr+size-(intptr_t)ret));
	}
	
	pmem_consmp += size; 

	/*插入文件标识列表*/
	filelist_t * filenow = (filelist_t *)malloc(sizeof(filelist_t));
	filenow->file_no = mmap_file;
	filenow->pool_paddr = ret;
	qr_new(filenow, link);
	qr_after_insert(pp->file, filenow, link);


	/*初始化双向链表freelist*/
	int freelist_len = PMEMPOOL_SIZE/chunksize; 
	freelist_t * pre = pp->fl_now;
	freelist_t * tmp;
	for (int i = 0; i < freelist_len; i++){
		tmp = (freelist_t *)malloc(sizeof(freelist_t));
		tmp->paddr = (void *)((intptr_t)ret+i*chunksize);
		qr_new(tmp, link);
		qr_after_insert(pre, tmp, link);
		pre = tmp;
	}	
	pp->fl_now = qr_next(pp->fl_now, link);
}

void pmempool_create(pmempool_t * pp)
{
	pp->fl_now = (freelist_t *)malloc(sizeof(freelist_t));
	pp->fl_now->paddr = NULL;
	qr_new(pp->fl_now, link);
	pp->file = (filelist_t *)malloc(sizeof(filelist_t));
	pp->file->file_no = 0;
	qr_new(pp->file, link);
	pmempool_create_one(pp);
}


/*unmap内存池*/
void pmempool_destroy(pmempool_t * pp)
{
	char str[32];

	filelist_t * tmp = qr_next(pp->file, link);
	while (tmp->file_no != 0)
	{
		//sprintf(str,"%s/%d", pmem_path, pp->file->file_no);
		sprintf(str,"/mnt/pmem/%d", pp->file->file_no);
		pmem_unmap(pp->file->pool_paddr, PMEMPOOL_SIZE);
		pmem_consmp -= PMEMPOOL_SIZE;
		remove(str);
	}
}

/*从mem pool中分配chunksize大小，和chunksize对齐*/
void * pmempool_chunk_alloc(pmempool_t * pp)
{	
	//如果内存池耗尽，则再分配一个
	if (qr_next(pp->fl_now, link) == pp->fl_now)
	{
		pmempool_create_one(pp);
		//return NULL;
	}

	if (pp->fl_now->paddr == NULL)  //空头结点
	{
		pp->fl_now = qr_next(pp->fl_now, link);
	}
	freelist_t * tmp = pp->fl_now;
	void * ret = tmp->paddr;
	pp->fl_now = qr_next(pp->fl_now, link);
	qr_remove(tmp, link);

	return ret;
}

/*将ptr所指空间返还给mem pool*/
void pmempool_free(pmempool_t * pp, void * ptr)
{
	freelist_t * tmp = (freelist_t *)malloc(sizeof(freelist_t));
	tmp->paddr = ptr;
	qr_new(tmp, link);
	qr_before_insert(pp->fl_now, tmp, link);
}
