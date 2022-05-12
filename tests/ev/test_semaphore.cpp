#undef NDEBUG
#include"Ev/Io.hpp"
#include"Ev/Semaphore.hpp"
#include"Ev/concurrent.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include<assert.h>
#include<string>

namespace {

/* Number of simultaneous threads the semaphore should allow.
 */
auto constexpr max = std::size_t(16);
/* Number of actual threads we will launch.  */
auto constexpr num_threads = std::size_t(5000);

/* Number of times we will Ev::yield() while within the
 * semaphore.
 */
auto constexpr busy_loop = std::size_t(20);

/* Keep track of the number of threads running in the
 * semaphore.  */
auto num_running = std::size_t(0);
/* Keep track of the number of times we actually
 * ran.
 */
auto num_completed = std::size_t(0);

auto hit_max = false;

/* Do nothing useful.  */
Ev::Io<void> be_busy(bool should_throw) {
	return Ev::lift().then([]() {
		++num_running;
		assert(num_running <= max);
		if (num_running == max)
			hit_max = true;
		return Ev::yield();
	}).then([]() {
		auto act = Ev::lift();
		for (auto i = std::size_t(0); i < busy_loop; ++i) {
			act += Ev::yield();
		}
		return act;
	}).then([]() {
		assert(num_running != 0);
		--num_running;
		return Ev::lift();
	}).then([should_throw]() {
		if (should_throw)
			throw std::exception();
		return Ev::lift();
	});
}

/* Run one thread.  */
Ev::Io<void> action(Ev::Semaphore& sem, bool should_throw) {
	return sem.run(be_busy(should_throw)).then([should_throw]() {
		assert(!should_throw);
		++num_completed;
		return Ev::lift();
	}).catching<std::exception>([should_throw](std::exception const&) {
		assert(should_throw);
		++num_completed;
		return Ev::lift();
	});
}

/* Run simultaneous threads.  */
Ev::Io<void> launch_loop(Ev::Semaphore& sem, std::size_t remaining) {
	return Ev::yield().then([&sem, remaining]() {
		if (remaining == 0)
			return Ev::lift();
		return Ev::concurrent(action(sem, (remaining % 2) == 0))
		     + launch_loop(sem, remaining - 1)
		     ;
	});
}
Ev::Io<void> launch_em_all(Ev::Semaphore& sem) {
	return launch_loop(sem, num_threads);
}

/* Wait for num_completed to reach target.  */
Ev::Io<void> wait_for_completion() {
	return Ev::yield().then([]() {
		if (num_completed < num_threads)
			return wait_for_completion();
		return Ev::lift();
	});
}

}

int main() {
	auto sem = Ev::Semaphore(max);

	auto code = Ev::lift().then([&]() {
		return launch_em_all(sem);
	}).then([]() {
		return wait_for_completion();
	}).then([]() {
		return Ev::yield(25);
	}).then([&]() {
		assert(hit_max);

		return sem.run(Ev::lift(std::string("this is a test")));
	}).then([&](std::string res) {
		assert(res == "this is a test");

		return sem.run(Ev::lift(0));
	});

	return Ev::start(code);
}
