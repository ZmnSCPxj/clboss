#undef NDEBUG
#include"basicsecure.h"
#include<assert.h>
#include<stdio.h>
#include<string.h>

#define SECRET "'Huffi-Muffi-Guffi'"

static void secret_using_function(int* retval) {
	char prebuf[200];
	char buffer[200];
	char postbuf[200];
	FILE* of;

	of = fopen("/dev/null", "w");

	strcpy(prebuf, "Not a secret.");
	strcpy(buffer, SECRET);
	strcpy(postbuf, "Still not a secret.");
	fprintf(of, "%s", prebuf);
	fprintf(of, "%s", buffer);
	fprintf(of, "%s", postbuf);

	/* Setting the below to #if 1 will cause this test to fail on
	 * GCC 10.2.0 at -O1 and -O2, so we know the test is a valid
	 * test for secret extrication.
	 * This also fails for CLANG 10.0.1 on FreeBSD 12.2 at -O3 when
	 * set to #if 1.
	 */
#if 0
	{
		int i;
		for (i = 0; i < sizeof(buffer); ++i)
			buffer[i] = 0;
	}
#else
	basicsecure_clear(buffer, sizeof(buffer));
#endif

	fclose(of);

	/* This is needed as otherwise CLANG on FreeBSD will tail-call
	 * into fclose above, which reuses the stack frame of this
	 * function, possibly clobbering the secrets that we want to
	 * see if it can be extricated.
	 * The below prevents the tail-call optimization, so that
	 * our stack frame remains sacrosanct and possibly still
	 * containing the secrets we want to see if they can be
	 * extricated.
	 */
	*retval = 0;
}

static inline void secret_extricating_function(int* retval) {
	char buffer[600];
	char const* ptr;

	if (*retval != 0)
		return;

	/* Do not run this test in valgrind, it is likely to complain.  */
	for ( ptr = buffer
	    ; ptr < buffer + sizeof(buffer) - 1 - sizeof(SECRET)
	    ; ++ptr
	    )
		assert(strncmp(ptr, SECRET, sizeof(SECRET) - 1));

	*retval = 0;
}

/* These pointers prevent inlining optimizations; the above
 * extricating function depends on its stack frame reusing
 * the same memory area as the secret_using_function, and
 * inlining one or both functions prevents their memory areas
 * from overlapping.
 */
void (*secret_using_function_ptr)(int*) = &secret_using_function;
void (*secret_extricating_function_ptr)(int*) = &secret_extricating_function;

int main() {
	int retval = 1;
	secret_using_function_ptr(&retval);
	secret_extricating_function_ptr(&retval);

	return retval;
}
