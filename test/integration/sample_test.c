#include <stdio.h>
#include <lsmalloc/lsmalloc.h>


int main(){
    printf("hello world!");
    void *ptr = lsmalloc(150,&ptr);

}