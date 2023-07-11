#undef NDEBUG
#include"Boss/Main.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Jsmn/Object.hpp"
#include"Jsmn/Parser.hpp"
#include"Net/Fd.hpp"
#include<assert.h>
#include<iostream>
#include<sstream>
#include<string>
#include<vector>

int main() {
	auto argv = std::vector<std::string>{"clboss"};

	/* Send a single getmanifest command using the new
	 * string command ID.
	 */
	auto cin = std::stringstream(R"JSON(
	{"id": "forty-two", "method": "getmanifest", "params": {}}
	)JSON");
	auto cout = std::stringstream("");
	auto cerr = std::stringstream("");

	auto dummy_open_rpc_socket = []( std::string const& lightning_dir
				       , std::string const& rpc_file
				       ){
		throw std::runtime_error("Should not be called");
		return Net::Fd();
	};

	auto main = Boss::Main( argv, cin, cout, cerr
			      , dummy_open_rpc_socket
			      );

	/* Run and check exit code.  */
	auto ec = Ev::start(main.run().then([](int ec) {
		return Ev::lift(ec);
	}));
	assert(ec == 0);

	std::cout << cout.str() << std::endl;

	/* Check the result of getmanifest.  */
	Jsmn::Parser parser;
	auto output = parser.feed(cout.str());
	/* If `dnsutils` is not installed, there will be a
	 * warning output.  */
	assert(output.size() == 1 || output.size() == 2);
	auto& last_output = output[output.size() - 1];
	assert(last_output.is_object());
	/* Make sure there is an "id" field that is "forty-two".  */
	assert(last_output.has("id"));
	assert(last_output["id"].is_string());
	assert(std::string(last_output["id"]) == "forty-two");

	return 0;
}
