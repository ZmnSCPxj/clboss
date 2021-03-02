/* This library was written by ZmnSCPxj and is placed in the public domain.  */
#undef NDEBUG
#include"basicsecure.h"
#include<assert.h>
#include<string.h>

/* VERY basic tests of functionality.  */
int main() {
	int a[999];
	int b[999];

	BASICSECURE_RAND(a, sizeof(a));
	BASICSECURE_RAND(b, sizeof(b));
	/* Highly unlikely.  */
	assert(!basicsecure_eq(a, b, sizeof(a)));

	memcpy(a, b, sizeof(a));
	assert(basicsecure_eq(a, b, sizeof(a)));

	basicsecure_clear(a, sizeof(a));
	assert(!basicsecure_eq(a, b, sizeof(a)));
	/* Smoke tests.  */
	assert(a[0] == 0);
	assert(a[998] == 0);

	basicsecure_clear(b, sizeof(b));
	assert(basicsecure_eq(a, b, sizeof(a)));

	return 0;
}

