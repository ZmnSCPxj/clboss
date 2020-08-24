#include<S/Bus.hpp>
#include<Util/make_unique.hpp>
#include<assert.h>
#include<unordered_map>

namespace S {

class Bus::Impl {
private:
	std::unordered_map< std::type_index
			  , std::unique_ptr<S::Detail::SignalBase>
			  > signals;

public:
	S::Detail::SignalBase&
	get_signal( std::type_index type
		  , std::function< std::unique_ptr<S::Detail::SignalBase>()
				 > make
		  ) {
		auto it = signals.find(type);
		if (it == signals.end()) {
			auto ins = signals.emplace(type, make());
			it = ins.first;
		}
		return *it->second;
	}
};

Bus::~Bus() { }
Bus::Bus() : pimpl(Util::make_unique<Impl>()) { }
Bus::Bus(Bus&& o) : pimpl(std::move(o.pimpl)) { }

S::Detail::SignalBase&
Bus::get_signal( std::type_index type
	       , std::function< std::unique_ptr<S::Detail::SignalBase>()
			      > make
	       ) {
	assert(pimpl);
	return pimpl->get_signal(type, std::move(make));
}

}
