#include"DnsSeed/Detail/parse_dig_srv.hpp"
#include"DnsSeed/get.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Ev/runcmd.hpp"
#include"Ev/yield.hpp"
#include<algorithm>
#include<iterator>
#include<memory>
#include<sstream>

namespace {

template<typename a>
std::string enstring(a const& val) {
	auto os = std::ostringstream();
	os << val;
	return os.str();
}

}

namespace DnsSeed {

Ev::Io<std::string> can_get() {
	return Ev::runcmd("dig", {"localhost"}).then([](std::string _) {
		return Ev::lift(std::string(""));
	}).catching<std::runtime_error>([](std::runtime_error const& _) {
		return Ev::lift(std::string(
			"`dig` command is not installed, it is usually "
			"available in a `dnsutils` package for your OS."
		));
	});
}

Ev::Io<std::vector<std::string>> get( std::string const& seed
				    ) {
	/* The @1.0.0.1 means we use the CloudFlare resolver to
	 * get DNS SRV queries.
	 * Some ISPs have default resolvers which do not properly
	 * handle SRV queries.
	 * It seems in practice using @1.0.0.1 helps in most such
	 * cases.
	 */
	return Ev::runcmd( "dig", {"@1.0.0.1", seed, "SRV"}
			 ).then([](std::string res) {
		auto records = Detail::parse_dig_srv(std::move(res));

		/* Generate the connect inputs.  */
		auto rv = std::vector<std::string>();
		std::transform( records.begin(), records.end()
			      , std::back_inserter(rv)
			      , [](DnsSeed::Detail::Record const& r) {
			return r.nodeid + "@" + r.hostname + ":"
			     + enstring(r.port)
			     ;
		});

		return Ev::lift(rv);
	});
}

}
