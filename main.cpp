#include<Ev/Io.hpp>
#include<Ev/start.hpp>
#include<Ev/yield.hpp>
#include<iostream>

namespace {

Ev::Io<int> io_main(int argc, char **argv) {
	return Ev::yield().then([]() {
		std::cout << "Hello World!" << std::endl;
		return Ev::lift(0);
	});
}

}

int main (int argc, char **argv) {
	return Ev::start(io_main(argc, argv));
}
