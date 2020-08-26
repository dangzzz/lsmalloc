#include "test/lsmalloc_test.h"


TEST_BEGIN(sampletest1)
{   
    /* test code */
	void * ret = huge_alloc(5000000);
	assert(ret != NULL);
	printf("%lld\n", (unsigned long long)ret);
	assert(ALIGNMEMT_CEILING(ret, chunksize) == ret);
	void * ret2 = huge_alloc(100000000);
	assert(ret2 != NULL);
	printf("%lld\n", (unsigned long long)ret2);
	assert(ALIGNMEMT_CEILING(ret2, chunksize) == ret2);
	huge_dalloc(ret);
	huge_dalloc(ret2);
}
TEST_END


TEST_BEGIN(sampletest2)
{
    /* test code */
    int x = 1;
    int y = 2;
    assert_d_le(x,y,"X should be less or equal to Y.");;
    assert_d_lt(x,y,"X should be less to Y.");
}
TEST_END

TEST_BEGIN(sampletest3)
{
    int x = 0;
    int *px = &x;
    int y = 0;
    int *py = &y;
    assert_ptr_ne(px,py,"Pointer px should not equal to pointer py.");
}
TEST_END




int
main(void)
{
	huge_boot();
	return (test(
	    sampletest1));
}
