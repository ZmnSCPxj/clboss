#include<Boss/Main.hpp>
#include<Boss/open_rpc_socket.hpp>
#include<Ev/Io.hpp>
#include<Ev/start.hpp>
#include<Net/Fd.hpp>
#include<iostream>
#include<memory>

namespace {

Ev::Io<int> io_main(int argc, char **argv) {
	auto arg_vec = std::vector<std::string>();
	for (int i = 0; i < argc; ++i) {
		arg_vec.push_back(std::string(argv[i]));
	}
	auto main_obj = std::make_shared<Boss::Main>(
		arg_vec, std::cin, std::cout, std::cerr,
		Boss::open_rpc_socket
	);
	return main_obj->run().then([main_obj](int ec) {
		/* Ensures main_obj is alive!  */
		return Ev::lift(ec);
	});
}

}

int main (int argc, char **argv) {
	/* For some reason, doing a direct Ev::start(io_main(argc, argv));
	 * results in a crash under an edge-case condition.
	 * This edge-case condition is if data is immediately fed to
	 * the stdin of clboss.
	 * You can tickle it (reverting to Ev::start(io_main(argc, argv))
	 * and recompiling first) with this:
	 *
	 *     echo '{"id": 0, "method": "getmanifest", "params": {}}' | ./clboss
	 *
	 * Note that if you just invoke clboss and input the above text
	 * manually, it will work perfectly fine, since most humans have
	 * reflex times measurable in dozens or hundreds of milliseconds.
	 *
	 * This does not normally happen in the usual case since the
	 * lightningd usually takes a bit of time before it actually
	 * emits the getmanifest call, and that is sufficient for the
	 * bug to leave the edge-case condition.
	 * On the other hand, a Sufficiently Fast (TM) processor might
	 * re-tickle this bug someday in the future even when run on
	 * some lightningd.
	 *
	 * This may be a bug in the C++ library or GCC itself, not
	 * CLBOSS.
	 * Why?
	 * Because the crash reports an error somewhere in the guts of
	 * malloc, and if you run clboss through valgrind, it does not
	 * crash even with the direct Ev::start(io_main(argc, argv)).
	 * valgrind works by replacing the malloc implementation, so ---
	 */
	auto code = io_main(argc, argv);
	return Ev::start(code);
}
