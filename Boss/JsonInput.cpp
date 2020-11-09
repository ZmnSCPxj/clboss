#include<assert.h>
#include"Boss/JsonInput.hpp"
#include"Boss/Msg/JsonCin.hpp"
#include"Ev/ThreadPool.hpp"
#include"Jsmn/Object.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"

namespace Boss {

class JsonInput::Impl {
private:
	Ev::ThreadPool& threadpool;
	std::istream& cin;
	S::Bus& bus;

public:
	Impl( Ev::ThreadPool& threadpool_
	    , std::istream& cin_
	    , S::Bus& bus_
	    ) : threadpool(threadpool_)
	      , cin(cin_)
	      , bus(bus_)
	      { }

	Ev::Io<void> run() {
		return threadpool.background<std::unique_ptr<Jsmn::Object>>([this]() {
			auto obj = Jsmn::Object();
			cin >> std::ws;
			if (!cin || cin.eof())
				return std::unique_ptr<Jsmn::Object>();
			cin >> obj;
			if (!cin || cin.eof())
				return std::unique_ptr<Jsmn::Object>();
			return Util::make_unique<Jsmn::Object>(std::move(obj));
		}).then([this](std::unique_ptr<Jsmn::Object> pobj) {
			if (!pobj)
				/* Exit loop.  */
				return Ev::lift();

			return bus.raise(Boss::Msg::JsonCin{
					std::move(*pobj)
			       })
			     + run()
			     ;
		});
	}
};

JsonInput::JsonInput( Ev::ThreadPool& threadpool
		    , std::istream& cin
		    , S::Bus& bus
		    ) : pimpl(Util::make_unique<Impl>(threadpool, cin, bus)) {}
JsonInput::~JsonInput() { }
JsonInput::JsonInput(JsonInput&& o) : pimpl(std::move(o.pimpl)) { }

Ev::Io<void> JsonInput::run() {
	assert(pimpl);
	return pimpl->run();
}

}
