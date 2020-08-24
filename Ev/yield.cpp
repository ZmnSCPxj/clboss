#include<Ev/Io.hpp>
#include<Ev/yield.hpp>
#include<Util/make_unique.hpp>
#include<ev.h>

namespace {

void yield_handler(EV_P_ ev_idle* raw_idler, int) {
	/* Acquire responsibility.  */
	auto idler = std::unique_ptr<ev_idle>(raw_idler);
	ev_idle_stop(EV_A_ idler.get());
	auto ppass = std::unique_ptr<std::function<void()>>(
		(std::function<void()>*) idler->data
	);

	/* Move off the pass function.  */
	auto pass = std::move(*ppass);

	/* Release resources.  */
	idler = nullptr;
	ppass = nullptr;

	pass();
}

}

namespace Ev {

Io<void> yield() {
	return Io<void>([]( std::function<void()> pass
			  , std::function<void(std::exception_ptr)> fail
			  ) {
		auto ppass = Util::make_unique<std::function<void()>>(
			std::move(pass)
		);

		auto idler = Util::make_unique<ev_idle>();
		ev_idle_init(idler.get(), &yield_handler);
		/* Release responsibility to C code.  */
		idler->data = ppass.release();
		ev_idle_start(EV_DEFAULT_ idler.release());
	});
}

}
