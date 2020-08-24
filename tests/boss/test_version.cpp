#undef NDEBUG
#include<Boss/Main.hpp>
#include<Ev/Io.hpp>
#include<Ev/start.hpp>
#include<assert.h>
#include<sstream>

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

	auto main = Boss::Main(argv, cin, cout, cerr);

	auto ec = Ev::start(main.run().then([]() {
		return Ev::lift(0);
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
