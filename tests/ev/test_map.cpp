#undef NDEBUG
#include"Ev/map.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include"Util/make_unique.hpp"
#include<assert.h>
#include<memory>
#include<stdexcept>
#include<vector>

int main() {
	auto flag = false;

	/* Functions.  */
	auto pi_to_pd = [](std::unique_ptr<int> pi) {
		auto ret = Util::make_unique<double>(*pi);
		return Ev::lift(std::move(ret));
	};
	auto fail_if_odd = [](int i) {
		if (i % 2)
			throw std::runtime_error("already odd");
		return Ev::lift(i + 1);
	};
	auto set_flag_if_called = [&flag](int i) {
		flag = true;
		return Ev::lift(i);
	};

	auto code = Ev::lift().then([&]() {

		/* Check move-only requirement.  */
		auto vec = std::vector<std::unique_ptr<int>>();
		vec.push_back(Util::make_unique<int>(0));
		vec.push_back(Util::make_unique<int>(1));
		vec.push_back(Util::make_unique<int>(2));
		return Ev::map(pi_to_pd, std::move(vec));
	}).then([&](std::vector<std::unique_ptr<double>> ovec) {
		assert(ovec.size() == 3);
		assert(*ovec[0] == 0.0);
		assert(*ovec[1] == 1.0);
		assert(*ovec[2] == 2.0);

		/* Check exception handling.  */
		flag = false;
		auto vec = std::vector<int>();
		vec.push_back(0);
		vec.push_back(1);
		vec.push_back(2);
		vec.push_back(3);
		return Ev::map(fail_if_odd, std::move(vec)
			      ).catching< std::runtime_error
				        >([&](std::runtime_error const& _) {
			flag = true;
			/* Return dummy vector.  */
			return Ev::lift(std::vector<int>());
		});
	}).then([&](std::vector<int> ovec) {
		assert(flag);

		/* Check empty vector.  */
		flag = false;
		auto vec = std::vector<int>();
		return Ev::map(set_flag_if_called, std::move(vec));
	}).then([&](std::vector<int> ovec) {
		assert(ovec.size() == 0);
		/* Nothing to process, so it should not have been called.  */
		assert(!flag);

		return Ev::lift(0);
	});

	return Ev::start(code);
}
