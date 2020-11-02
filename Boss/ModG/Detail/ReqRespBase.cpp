#include"Boss/ModG/Detail/ReqRespBase.hpp"
#include"Boss/Shutdown.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Util/make_unique.hpp"

namespace Boss { namespace ModG { namespace Detail {

ReqRespBase::~ReqRespBase() {
	auto my_entries = std::move(entries);
	for (auto& e : my_entries) {
		auto fail = std::move(e.second->fail);
		e.second = nullptr;
		try {
			throw Boss::Shutdown();
		} catch(...) {
			fail(std::current_exception());
		}
	}
}


Ev::Io<std::shared_ptr<void>>
ReqRespBase::execute( std::shared_ptr<void> request
		    , std::function<void( std::shared_ptr<void> const&
				   , void*
				   )> const& set_annotation
		    , std::function<Ev::Io<void>(std::shared_ptr<void>)
				   > const& broadcast
		    ) {
	return Ev::lift().then([ this
			       , request
			       , set_annotation
			       , broadcast
			       ]() {
		auto pentry = Util::make_unique<Entry>();
		pentry->ready = false;
		auto key = pentry.get();
		auto& entry = *pentry;
		entries[key] = std::move(pentry);
		set_annotation(request, (void*) key);

		return Boss::concurrent( broadcast(request)
				       ).then([&entry]() {
			return Ev::Io<std::shared_ptr<void>
				     >([&entry
				       ](PassF pass, FailF fail) {
				entry.pass = std::move(pass);
				entry.fail = std::move(fail);
				entry.ready = true;
			});
		});
	}).then([](std::shared_ptr<void> response) {
		return Ev::yield().then([response]() {
			return Ev::lift(response);
		});
	});
}

Ev::Io<void>
ReqRespBase::on_response( std::shared_ptr<void> response
			, std::function<void*(std::shared_ptr<void> const&)
				       > const& get_annotation
			) {
	auto key = get_annotation(response);
	auto it = entries.find(key);
	if (it == entries.end())
		return Ev::lift();

	auto& entry = *it->second;
	return wait_entry_ready(entry).then([ this
					    , response
					    , it
					    , &entry
					    ]() {
		auto pass = std::move(entry.pass);
		entries.erase(it);
		pass(response);
		return Ev::lift();
	});
}
Ev::Io<void>
ReqRespBase::wait_entry_ready(Entry& entry) {
	return Ev::yield().then([&entry]() {
		if (entry.ready)
			return Ev::lift();
		return wait_entry_ready(entry);
	});
}

}}}
