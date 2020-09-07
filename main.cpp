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
	return Ev::start(io_main(argc, argv));
}
