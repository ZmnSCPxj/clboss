#include"Boss/Mod/all.hpp"
#include<vector>

namespace {

class All {
private:
	std::vector<std::shared_ptr<void>> modules;

public:
	template<typename M, typename... As>
	void install(As&&... as) {
		auto ptr = std::make_shared<M>(as...);
		modules.push_back(std::shared_ptr<void>(std::move(ptr)));
	}
};

class Dummy {
};

}

namespace Boss { namespace Mod {

std::shared_ptr<void> all( std::ostream& cout
			 , S::Bus& bus
			 , Ev::ThreadPool& threadpool
			 ) {
	auto all = std::make_shared<All>();

	all->install<Dummy>();

	return all;
}

}}
