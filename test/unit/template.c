#include "test/lsmalloc_test.h"


TEST_BEGIN(sampletest1)
{   
    /* test code */
    bool x = true;
    assert_true(x,"This value should be true.");
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

	return (test(
	    sampletest1,
        sampletest2,
        sampletest3));
}