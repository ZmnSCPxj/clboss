#undef NDEBUG
#include<Boss/Main.hpp>
#include<Ev/Io.hpp>
#include<Ev/start.hpp>
#include<Jsmn/Object.hpp>
#include<Jsmn/Parser.hpp>
#include<Net/Fd.hpp>
#include<assert.h>
#include<iostream>
#include<set>
#include<sstream>
#include<stdexcept>

/* Test that getmanifest causes Boss::Main to emit
 * the commands and options we expect.
 */
namespace {
auto const expected_commands = std::vector<std::string>
{ "clboss-status"
, "clboss-externpay"
, "clboss-ignore-onchain"
, "clboss-notice-onchain"
, "clboss-unmanage"
, "clboss-swaps"
};
auto const expected_options = std::vector<std::string>
{ "clboss-min-onchain"
, "clboss-auto-close"
, "clboss-zerobasefee"
, "clboss-min-channel"
, "clboss-max-channel"
};
}

int main() {
	auto argv = std::vector<std::string>{"clboss"};

	/* Send a single getmanifest command.  */
	auto cin = std::stringstream(R"JSON(
	{"id": 0, "method": "getmanifest", "params": {}}
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

	auto ec = Ev::start(main.run().then([](int ec) {
		return Ev::lift(ec);
	}));
	assert(ec == 0);

	std::cout << cout.str() << std::endl;

	/* Check the result of getmanifest.  */
	Jsmn::Parser parser;
	auto output = parser.feed(cout.str());
	/* Make sure there is exactly one object emitted.  */
	assert(output.size() == 1);
	/* Make sure it is an object.  */
	auto& output0 = output[0];
	assert(output0.is_object());
	/* Make sure it has a "result" key that is an object.  */
	assert(output0.has("result"));
	auto result = output0["result"];
	assert(result.is_object());
	/* Make sure result has "rpcmethods" and "options" keys that
	 * are arrays.  */
	assert(result.has("rpcmethods"));
	assert(result.has("options"));
	auto j_rpcmethods = result["rpcmethods"];
	auto j_options = result["options"];
	assert(j_rpcmethods.is_array());
	assert(j_options.is_array());

	/* Generate the actual set of commands and options.  */
	auto actual_commands = std::set<std::string>();
	auto actual_options = std::set<std::string>();
	for (auto cmd : j_rpcmethods) {
		/* Each command should be an object with
		 * key "name".  */
		assert(cmd.is_object());
		assert(cmd.has("name"));
		auto name = cmd["name"];
		/* The name should be a string.  */
		assert(name.is_string());
		/* Add it to the set of actual commands.  */
		actual_commands.emplace(std::string(name));
	}
	for (auto opt : j_options) {
		/* Each option should be an object with
		 * key "name".  */
		assert(opt.is_object());
		assert(opt.has("name"));
		auto name = opt["name"];
		/* The name should be a string.  */
		assert(name.is_string());
		/* Add it to the set of actual options.  */
		actual_options.emplace(std::string(name));
	}

	/* Now check the expected commands and options exist.  */
	for (auto const& cmd : expected_commands) {
		assert(actual_commands.find(cmd) != actual_commands.end());
	}
	for (auto const& opt : expected_options) {
		assert(actual_options.find(opt) != actual_options.end());
	}

	return 0;
}
