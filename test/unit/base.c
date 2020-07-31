#include "test/lsmalloc_test.h"

TEST_BEGIN(basetest)
{
	void * a = base_alloc(100);
	printf("%llu\n", (unsigned long long)a);
}
TEST_END

int 
main(void)
{
	chunk_boot();
	base_boot();
	return test(basetest);
}
