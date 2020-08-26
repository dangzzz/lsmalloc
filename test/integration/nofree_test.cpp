#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <random>
#include <string.h>
#include <iostream>
#include "lsmalloc/lsmalloc.h"
#include <sys/time.h>
using namespace std;

typedef unsigned long long ull;


default_random_engine rand_e;
std::normal_distribution<double> nor_b1(100, 50);



const ull max_alloc_sz = 100ULL*1024*1024; //100MB



const int size_500m=5242881*4; 
void * rd_addr[size_500m];
ull rd_sz[size_500m];

ull alloc_object_num;  //number of live objects
ull alloc_sz; //total memory we have allocated(include freed)





/*allocate one object, size=sz*/
void malloc_one(ull sz)
{
	rd_addr[alloc_object_num] = lsmalloc(sz,&(rd_addr[alloc_object_num]));
	memset(rd_addr[alloc_object_num], -1, sz);
	rd_sz[alloc_object_num] = sz;
	alloc_object_num++;
	alloc_sz += sz;
	//printf("malloc one success\n");
}





ull get_size()
{
	return lround(nor_b1(rand_e));
}




void test()
{
	alloc_sz = 0; 
	alloc_object_num = 0;


	/***************************NOFREE****************************/

	printf("\n----NOFREE-----\n");
	while (alloc_sz < max_alloc_sz)
	{

		
		ull sz = get_size();
		while(sz>10000){
			sz = get_size();
		}
		if (alloc_sz + sz > max_alloc_sz)
		{
			break;
		}

		malloc_one(sz);
		
	}	

}

int main(int argc, char *argv[])
{	
	lspmemdir("/mnt/pmemdir/");
    srand((unsigned)time(NULL));

	struct timeval start,end;  
	gettimeofday(&start, NULL );  
	//…calculating…  

	test();


	gettimeofday(&end, NULL );  
	long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;  
	printf("time=%f\n",timeuse /1000000.0);  
	

	return 0;
	
}
