#define	LSMALLOC_POOL_C_
#include "lsmalloc/internal/lsmalloc_internal.h"
//todo per arena has its own pool
/******************************************************************************/
/* Data. */
extern int mmap_file; 
extern int pmem_consmp;

/******************************************************************************/
/*
 * Function prototypes for static functions that are referenced prior to
 * definition.
 */



/******************************************************************************/
/* Inline tool function */


/******************************************************************************/
/*mmap一个memory pool*/
void pmempool_create(pmempool_t * pp){
	void * ret, *addr;
	char str[32];
	size_t mapped_len;
	int is_pmem;

	size_t size = PMEMPOOL_SIZE;
	size_t alignment = chunksize;  

	sprintf(str,"/mnt/pmem/%d",mmap_file);
	mmap_file++;

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
	
	((pmempool_t *)ret)->file_no = mmap_file-1;
	pmem_consmp += size; 
	pp->addr = ret;

	/*初始化双向链表freelist*/
	for (int i = 0; i < freelist_len; i++){
		pp->freelist[i].nxt = i+1;
	}	
	for (int i = 1; i <= freelist_len; i++){
		pp->freelist[i].pre = i-1;
	}
	pp->freelist[freelist_len].nxt = 0;
	pp->freelist[0].pre= freelist_len;
	pp->fl_now = 0;
}


/*unmap内存池*/
void pmempool_destroy(pmempool_t * pp){
	char str[32];

	sprintf(str,"/mnt/pmem/%d",pp->file_no);
	pmem_unmap(pp->addr, PMEMPOOL_SIZE);
	pmem_consmp -= PMEMPOOL_SIZE;
	remove(str);
}


/*从mem pool中分配chunksize大小，和chunksize对齐*/
void * pmempool_chunk_alloc(pmempool_t * pp){
	
	//内存池耗尽
	if (pp->freelist[pp->fl_now].pre == freelist_len && 
			pp->freelist[pp->fl_now].nxt == freelist_len){
		return NULL;
	}

	void * ret = (void *)((intptr_t)pp->addr + pp->fl_now*chunksize); 

	pp->freelist[pp->freelist[pp->fl_now].pre].nxt = pp->freelist[pp->fl_now].nxt;
	pp->freelist[pp->freelist[pp->fl_now].nxt].pre = pp->freelist[pp->fl_now].pre;
	pp->fl_now = pp->freelist[pp->fl_now].nxt;
	
	if (pp->fl_now == freelist_len){
		pp->fl_now = pp->freelist[pp->fl_now].nxt;
	}
	return ret;
}

/*将ptr所指空间返还给mem pool*/
void pmempool_free(pmempool_t * pp, void * ptr){
	int id = ((intptr_t)ptr-(intptr_t)pp->addr)/chunksize;
	pp->freelist[id].pre = pp->freelist[pp->fl_now].pre;
	pp->freelist[id].nxt = pp->fl_now;
	pp->freelist[pp->freelist[pp->fl_now].pre].nxt = id;
	pp->freelist[pp->fl_now].pre = id;
}
