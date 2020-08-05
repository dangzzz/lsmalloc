#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <random>
#include <string.h>
#include <iostream>
#include "lsmalloc/lsmalloc.h"

using namespace std;

typedef unsigned long long ull;

const ull max_live_sz = 100ULL*1024*1024; //100MB
const ull max_alloc_sz = 500ULL*1024*1024;  //500MB


const int size_500m=5242881*4; 
const int size_1g=10737429*2;
const int size_5g=53687092*2;
const int size_10g=107374183*2;

void * rd_addr[size_500m];
ull rd_sz[size_500m];

ull alloc_object_num;  //number of live objects
ull alloc_sz; //total memory we have allocated(include freed)
ull live_sz; //live memory (alloc_sz minus freed)


ull getRSS()
{
	FILE * fstats = fopen("/proc/self/statm", "r");
	//the file contains 7 data:
	//vmsize vmrss shared text lib data dt
	
    size_t buffsz = 0x1000;
	char buff[buffsz];
	buff[buffsz-1] = 0;
	fread( buff, 1, buffsz-1, fstats);
	fclose( fstats);
	const char* pos = buff;
	
	//get "vmrss"
	while ( *pos && *pos == ' ' ) ++pos;
	while ( *pos && *pos != ' ' ) ++pos;
	ull rss = atol(pos);
	
	//get "shared"
	while ( *pos && *pos == ' ' ) ++pos;
	while ( *pos && *pos != ' ' ) ++pos;
	ull shared = atol(pos);
	//ull shared = 0; 
	return (rss-shared)*4*1024; //B
}



/*allocate one object, size=sz*/
void malloc_one(ull sz)
{
	rd_addr[alloc_object_num] = lsmalloc(sz,&(rd_addr[alloc_object_num]));
	memset(rd_addr[alloc_object_num], -1, sz); /////
	//rd_addr.push_back(tmp);
	//rd_sz.push_back(sz);
	rd_sz[alloc_object_num] = sz;
	alloc_object_num++;
	alloc_sz += sz;
	live_sz += sz;
}


/*Free one object, index=x*/
void free_one(ull x)
{
	live_sz -= rd_sz[x];
	free(rd_addr[x]);
	rd_sz[x] = 0; //=0 means freed
}

//generate uniform distribution
//need compile option: -std=c++11
default_random_engine rand_e;
uniform_int_distribution<ull> uni_d1(64, 256);
uniform_int_distribution<ull> uni_d2(100, 200);
uniform_int_distribution<ull> uni_d3(1000, 2000);
uniform_int_distribution<ull> uni_d4(50, 150);
std::normal_distribution<double> nor_b1(250, 0.1);

int test_flag; //which overhead for test: 1~8

ull get_size_before()
{
	switch (test_flag)
	{
		case 1: return lround(nor_b1(rand_e));break;
		case 2: return lround(nor_b1(rand_e));break;
		case 3: return 100;break;
		case 4: return uni_d1(rand_e);break;
		case 5: return uni_d1(rand_e);break;
		case 6: return uni_d2(rand_e);break;
		case 7: return uni_d3(rand_e);break;
		case 8: return uni_d4(rand_e);break;
	}
}

uniform_int_distribution<ull> uni_d5(64, 256);
uniform_int_distribution<ull> uni_d6(1000, 2000);
uniform_int_distribution<ull> uni_d7(1500, 2500);
uniform_int_distribution<ull> uni_d8(5000, 15000);
std::normal_distribution<double> nor_a1(64, 20);
ull get_size_after()
{
	switch (test_flag)
	{
		case 2: return lround(nor_a1(rand_e));break;
		case 3: return 130;break;
		case 4: return uni_d5(rand_e);break;
		case 5: return uni_d5(rand_e);break;
		case 6: return uni_d6(rand_e);break;
		case 7: return uni_d7(rand_e);break;
		case 8: return uni_d8(rand_e);break;
	}
}


ull test()
{
	ull maxRSS = 0;
	ull cnt = 0; //print some message
	alloc_sz = 0; 
	live_sz = 0;
	alloc_object_num = 0;

	FILE * fp = fopen("alloc_test.result", "w");

	/***************************Phase1 : Before*****************************/

	printf("\n----Phase1-----\n");
	while (alloc_sz < max_alloc_sz)
	{

		
		ull sz = get_size_before();
		while(sz>10000){
			sz = get_size_before();
		}
		if (live_sz + sz > max_live_sz)
		{
			break;
		}

		malloc_one(sz);
		
		ull nowRSS = getRSS();
		maxRSS = max(maxRSS, nowRSS);
		/*
		if (++cnt % 100 == 0)
		{
			fprintf(fp, "--phase1 alloc_sz:%12llu nowRSS:%12llu maxRSS:%12llu\n", alloc_sz, nowRSS, maxRSS);
			cnt = 0;
		}
		*/
	}	

	if (test_flag == 1) 
	{
		fclose(fp);
		return maxRSS;
	}
	return maxRSS;
	/***************************Phase2 : Delete*****************************/

	printf("\n----Phase2-----\n");
	ull max_del_sz = 0;
	switch(test_flag)
	{
		case 2: break;
		case 3: max_del_sz = live_sz * 0.9;break;
		case 4: break;
		case 5: max_del_sz = live_sz * 0.9;break;
		case 6: max_del_sz = live_sz * 0.9;break;
		case 7: max_del_sz = live_sz * 0.5;break;
		case 8: max_del_sz = live_sz * 0.9;break;
		case 9: max_del_sz = live_sz * 0.9;break;
	}
	
	ull del_sz = 0;
	ull x = 0;
    ull	len = alloc_object_num; //len: number of objects allocated in Phase 1
	while(x < len && del_sz < max_del_sz)
	{
//		printf("%llu\n", x);
		if (rd_sz[x] != 0) 
		{
			del_sz = rd_sz[x];
			free_one(x);
		}
		x++;
	}

	/***************************Phase3 : After*****************************/

	printf("\n----Phase3-----\n");
	fprintf(fp, "-------------------------------------phase3-----------------------------------\n");
	alloc_sz = 0;
	while (alloc_sz < max_alloc_sz)
	{
//		if (++cnt % 100 == 0)
//		{
//			printf("--%llu al=%llu live=%llu %llu\n", cnt, alloc_sz, live_sz, getRSS());
//		}

		ull sz = get_size_after();
		while(sz>10000){
			sz = get_size_after();
		}
		/*we need to replace the allocted objects in phase1,
		*so firstly delete from the head.
	    *delete randomly after all objects in phase1 are freed. */
		while (live_sz + sz > max_live_sz)
		{
			if (x >= len) //have freed all memory allocated in phase1
			{
				ull id = len+rand()%(alloc_object_num-len);
				if (rd_sz[id] != 0) free_one(id); 
			}
			else
			{	
				if (rd_sz[x] != 0) free_one(x);
				x++;
			}
		}
		
		malloc_one(sz);

		ull nowRSS = getRSS();
		maxRSS = max(maxRSS, nowRSS);
		/*
		if (++cnt %100 == 0)
		{
			fprintf(fp, "--phase3 alloc_sz:%12llu nowRSS:%12llu maxRSS:%12llu\n", alloc_sz, nowRSS, maxRSS);
			cnt = 0;
		}
		*/
	}	
	
	fclose(fp);
	return maxRSS;	
}

int main(int argc, char *argv[])
{	/*
	std::default_random_engine e; //引擎
	std::normal_distribution<double> n(256, 3); //均值, 方差
	std::vector<unsigned> vals(280);
	for(std::size_t i=0; i != 2000; ++i) {
		unsigned v = std::lround(n(e)); //取整-最近的整数
		if (v < vals.size())
			++vals[v];
	}
	for (std::size_t j=0; j != vals.size(); ++j)
		std::cout << j << " : " << vals[j] << std::string(vals[j], '*') << std::endl;
 
	int sum = std::accumulate(vals.begin(), vals.end(), 0);
	std::cout << "sum = " << sum << std::endl;
	return 0;
*/
    srand((unsigned)time(NULL));
	memset(rd_addr, -1, sizeof(rd_addr)); //useless
	memset(rd_sz, -1, sizeof(rd_sz)); //useless
	rd_addr[rand()%100000] = &rd_sz[rand()%1000000]; //useless 
	rd_sz[rand()%100000] = 10; //useless
	printf("%llu\n", getRSS()); //useless

	
	test_flag = argv[1][0] - '0'; //'1' --> 1
	ull rdsize = sizeof(rd_addr) + sizeof(rd_sz); 
	ull maxrss = test();
	printf("rd size = %llu  maxRSS = %llu  maxRSS-rdsize = %llu\n", rdsize, maxrss, maxrss-rdsize);

	//free all memory
	ull len = alloc_object_num;
	for (int i = 0; i < len; i++)
	{

		if (rd_sz[i] != 0) free(rd_addr[i]);
	}

	return 0;
	
}
