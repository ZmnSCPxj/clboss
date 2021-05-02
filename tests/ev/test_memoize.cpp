#undef NDEBUG
#include"Ev/memoize.hpp"
#include"Ev/start.hpp"
#include<assert.h>
#include<cstddef>
#include<cstdint>

int main() {

	auto add_func_calls = std::size_t(0);
	auto add_func = [&add_func_calls](std::uint64_t a, std::uint64_t b) {
		return Ev::lift().then([&add_func_calls, a, b]() {
			++add_func_calls;
			return Ev::lift(std::size_t(a + b));
		});
	};
	auto add_func_memo = Ev::memoize(add_func);

	auto code = Ev::lift().then([&]() {

		add_func_calls = 0;
		return add_func_memo(1, 2);
	}).then([&](std::uint64_t res) {
		/* Data flows through correctly?  */
		assert(res == std::uint64_t(3));
		/* Called exactly once?  */
		assert(add_func_calls == 1);

		/* Call memoized version again, which should return
		 * the same result but without calling the core
		 * function.
		 */
		return add_func_memo(1, 2);
	}).then([&](std::uint64_t res) {
		assert(res == std::uint64_t(3));
		/* Number of calls should not have increased!  */
		assert(add_func_calls == 1);

		/* Call memoized version with other arguments.  */
		return add_func_memo(2, 1);
	}).then([&](std::uint64_t res) {
		assert(res == std::uint64_t(3));
		/* Number of calls should have increased.  */
		assert(add_func_calls == 2);

		/* Call again with previous arguments.  */
		return add_func_memo(1, 2);
	}).then([&](std::uint64_t res) {
		assert(res == std::uint64_t(3));
		/* Number of calls should not have increased.  */
		assert(add_func_calls == 2);

		/* A copy of the memoized function works the same as the
		 * memoized function.
		 */
		auto add_func_memo_copy = add_func_memo;
		return add_func_memo_copy(2, 1);
	}).then([&](std::uint64_t res) {
		assert(res == std::uint64_t(3));
		/* Number of calls should not have increased.  */
		assert(add_func_calls == 2);

		return Ev::lift(0);
	});

	return Ev::start(code);
}
