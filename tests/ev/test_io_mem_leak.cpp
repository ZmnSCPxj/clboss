#undef NDEBUG
#include<new>
#include<stdlib.h>

/* Memory leak detection.  */
unsigned long count = 0;

void* operator new(unsigned long size) {
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
void operator delete(void* p, unsigned long) noexcept {
	--count;
	free(p);
}

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
