#include <stdio.h>
#include <lsmalloc/lsmalloc.h>


int main(){
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("hello world!\n");

    lspmemdir("/mnt/pmem/");
    printf("1\n");
    void *ptr = lsmalloc(320,&ptr);
    printf("2\n");
    return 0;

}