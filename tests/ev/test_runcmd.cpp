#undef NDEBUG
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ev/runcmd.hpp"
#include<assert.h>
#include<stdexcept>

int main() {
	auto flag = false;

	auto action = Ev::lift().then([]() {
		return Ev::runcmd("cat", {});
	}).then([](std::string cat) {
		/* Should be an empty string, since input is /dev/null.  */
		assert(cat == "");

		/* Test on large output.  */
		return Ev::runcmd("sh", {"-c", "ls -la *"});
	}).then([&flag](std::string result) {
		/* Should be nonempty.  */
		assert(result.size() != 0);

		/* Test on non-existent command.  */
		return Ev::runcmd("test-on-nonexistent-command", {})
				.catching<std::runtime_error>([&flag](std::runtime_error const& e) {
			/* Ignore error, set flag.  */
			flag = true;
			return Ev::lift(std::string(""));
		});
	}).then([&flag](std::string result) {
		assert(flag);
		assert(result == "");

		return Ev::lift();
	}).then([]() {
		return Ev::lift(0);
	});

	return Ev::start(action);
}
