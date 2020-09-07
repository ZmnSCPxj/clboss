#undef NDEBUG
#include<Boss/Main.hpp>
#include<Ev/Io.hpp>
#include<Ev/start.hpp>
#include<Net/Fd.hpp>
#include<assert.h>
#include<sstream>
#include<stdexcept>

#ifdef HAVE_CONFIG_H
# include"config.h"
#endif

/* Test that --version causes Boss::Main to exit immediately
 * after printing the version.  */
int main() {
	auto argv = std::vector<std::string>{ "clboss", "--version" };

	auto cin = std::stringstream("dummy");
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
	/* No input should be consumed.  */
	assert(cin.str() == "dummy");
	/* Should just print this.  */
	assert(cout.str() == (PACKAGE_STRING "\n"));
	/* No error should have been printed.  */
	assert(cerr.str() == "");

	return 0;
}
