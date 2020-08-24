#include<Ev/Io.hpp>
#include<Ev/start.hpp>
#include<Util/make_unique.hpp>
#include<ev.h>
#include<iostream>
#include<memory>

namespace {

/* Contains the Io<int> *and* the exit code.  */
struct MainContainer {
	int exit_code;
	Ev::Io<int> main;
};

void start_idle_handler(EV_P_ ev_idle *raw_idler, int revents) {
	/* Recover control of idler back from C.  */
	auto idler = std::unique_ptr<ev_idle>(raw_idler);
	ev_idle_stop(EV_A_ idler.get());

	auto containerp = (MainContainer*) idler->data;
	auto main = std::move(containerp->main);

	main.run([containerp](int exit_code) {
		containerp->exit_code = exit_code;
	}, [containerp](std::exception_ptr e) {
		try {
			std::rethrow_exception(e);
		} catch (std::exception const& e) {
			std::cerr << "Unhandled exception!" << std::endl;
			std::cerr << e.what() << std::endl;
		} catch (...) {
			std::cerr << "Unhandled exception of unknown type!"
				  << std::endl;
		}

		containerp->exit_code = 254;
	});
}

}

namespace Ev {

int start(Io<int> main) {
	if (!ev_default_loop()) {
		std::cerr << "libev failed to initialize" << std::endl;
		return 255;
	}

	auto container = MainContainer{ 255, std::move(main) };

	auto idler = Util::make_unique<ev_idle>();
	ev_idle_init(idler.get(), &start_idle_handler);
	idler->data = &container;

	/* Give control over to C code.  */
	ev_idle_start(EV_DEFAULT_ idler.release());

	/* Run mainloop.  */
	auto result = ev_run(EV_DEFAULT_UC);
	if (result) {
		/* Just warn.  */
		std::cerr << "WARNING: libev waiters still active." << std::endl;
	}

	return container.exit_code;
}

}
