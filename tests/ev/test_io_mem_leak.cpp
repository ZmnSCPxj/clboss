#undef NDEBUG
#include<new>
#include<stddef.h>
#include<stdlib.h>

unsigned long count = 0;

/* Memory leak detection.  */
#if !USE_VALGRIND || TEST_IO_MEM_LEAK_ALLOW_VALGRIND
/* In some systems (Ubuntu 18.04 maybe?), valgrind will override the
 * operator new below, but will not override the operator delete,
 * leading to allocating using valgrind's operator new() but freeing
 * with free() due to the code below.
 *
 * So, we disable this code if USE_VALGRIND.
 *
 * On systems where we *know* valgrind works correctly, we can give
 * `./configure CXXFLAGS="-DTEST_IO_MEM_LEAK_ALLOW_VALGRIND"` to
 * still perform this counting test even with valgrind
 *
 * This test is stricter than what valgrind tests --- this test
 * ensures we do not have "live" leaks, i.e. allocating more and more
 * objects while keeping pointers to them, which valgrind will ignore
 * since there are pointers to the memory, but which should not
 * happen since semantically those objects are dead.
 */

void* operator new(size_t size) {
	++count;
	return malloc(size);
}
void operator delete(void* p) noexcept {
	--count;
	free(p);
}
/* For some reason, GCC 9, on -O2, uses this function to
 * delete an object it allocated with the above operator
 * new, which massively confuses our count (and raises
 * an error in valgrind).
 * It is not used on GCC 9 on -O0 though....
 */
void operator delete(void* p, size_t) noexcept {
	--count;
	free(p);
}
#endif /* !USE_VALGRIND */

#include<assert.h>
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"

#define NUM_LOOPS 200
#define MAX_COUNT 10

Ev::Io<int> loop(unsigned long n, bool& flag) {
	return Ev::yield().then([n, &flag]() {
		/* Confirm the loop executed at least once.  */
		flag = true;
		/* The number of allocated objects should remain small.  */
		assert(count < MAX_COUNT);
		if (n == 0) {
			return Ev::lift(0);
		} else {
			return loop(n - 1, flag);
		}
	});
}

int main() {
	auto flag = bool(false);
	auto ec = Ev::start(loop(NUM_LOOPS, flag));
	assert(flag);
	return ec;
}
