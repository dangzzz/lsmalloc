#include <stdio.h>
#include <lsmalloc/lsmalloc.h>


int main(){
    printf("hello world!");

    lspmemdir("path to pmem");
    void *ptr = lsmalloc(150,&ptr);

}