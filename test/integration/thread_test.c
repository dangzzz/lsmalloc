
#include <stdio.h>
#include "test/lsmalloc_test.h"
#include <stdlib.h>

int niterations = 50;
int nobjects = 1000;
int work = 0;
int sz = 40;
int nthread = 10;


struct Foo
{
    int x;
    int y;
};

void *worker(void *arg)
{
    int i, j;
    struct Foo **a;

    a = (struct Foo **)malloc(8 * nobjects);
    for (j = 0; j < niterations; j++)
    {

        // printf ("%d\n", j);
        for (i = 0; i < (nobjects); i++)
        {
            a[i] = (struct Foo *)lsmalloc(sz * sizeof(struct Foo), (void**)&a[i]);

            for (volatile int d = 0; d < work; d++)
            {
                volatile int f = 1;
                f = f + f;
                f = f * f;
                f = f + f;
                f = f * f;
            }
            assert(a[i]);
        }

        for (i = 0; i < (nobjects); i++)
        {

            lsfree(a[i]);

            for (volatile int d = 0; d < work; d++)
            {
                volatile int f = 1;
                f = f + f;
                f = f * f;
                f = f + f;
                f = f * f;
            }
        }
    }

    free(a);

    return NULL;
}

int main()
{
    //single thread OK
    //worker(NULL);
    //return 0;

     
    pthread_t tids[nthread]; //线程的id
    for(int i=0;i<nthread;++i)
    {
        pthread_create(&tids[i], NULL, worker, NULL);
    }
    for(int i=0;i<nthread;++i)
    {
        pthread_join(tids[i],NULL);
    }
    return 0;
}