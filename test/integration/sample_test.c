#include <stdio.h>
#include <lsmalloc/lsmalloc.h>


int main(){
    printf("hello world!");

    lspmemdir("/mnt/pmemdir/");
    void *ptr = lsmalloc(150,&ptr);

}