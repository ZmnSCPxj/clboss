#include"DnsSeed/Detail/parse_dig_srv.hpp"
#include"DnsSeed/get.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Ev/runcmd.hpp"
#include"Ev/yield.hpp"
#include<arpa/inet.h>
#include<algorithm>
#include<errno.h>
#include<memory>
#include<netdb.h>
#include<sstream>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>

namespace {

class AddrInfoReleaser {
private:
	addrinfo* addrs;
public:
	AddrInfoReleaser() : addrs(nullptr) { }
	AddrInfoReleaser(AddrInfoReleaser const&) =delete;
	AddrInfoReleaser(AddrInfoReleaser&& o) {
		addrs = o.addrs;
		o.addrs = nullptr;
	}

	~AddrInfoReleaser() {
		if (addrs)
			freeaddrinfo(addrs);
	}

	addrinfo*& get() { return addrs; }
};

template<typename a>
std::string enstring(a const& val) {
	auto os = std::ostringstream();
	os << val;
	return os.str();
}

std::string resolve(std::string const& host, std::uint16_t port) {
	auto port_string = enstring(port);

	auto hint = addrinfo();
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = 0;
	hint.ai_flags = AI_ADDRCONFIG;

	auto addrs = AddrInfoReleaser();
	auto res = getaddrinfo( host.c_str(), port_string.c_str()
			      , &hint
			      , &addrs.get()
			      );
	if (res != 0)
		throw std::runtime_error(
			std::string("DnsSeed::get: getaddrinfo: ") +
			gai_strerror(res)
		);

	for (auto p = addrs.get(); p; p = p->ai_next) {
		void *addr;
		if ( p->ai_family == AF_INET
		  && p->ai_addrlen == sizeof(sockaddr_in)
		   ) {
			auto sa = reinterpret_cast<sockaddr_in*>(p->ai_addr);
			addr = &sa->sin_addr;
		} else if ( p->ai_family == AF_INET6
			 && p->ai_addrlen == sizeof(sockaddr_in6)
			  ) {
			auto sa6 = reinterpret_cast<sockaddr_in6*>(p->ai_addr);
			addr = &sa6->sin6_addr;
		} else
			continue;

		char buf[INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN];
		auto iptxt = inet_ntop(p->ai_family, addr, buf, sizeof(buf));
		if (!iptxt)
			continue;
		return std::string(iptxt);
	}

	return host;
}

/* Performs resolution for each of the discovered nodes.  */
class MultiGetAddrInfo {
private:
	Ev::ThreadPool& threadpool;
	std::vector<DnsSeed::Detail::Record> records;
	std::vector<DnsSeed::Detail::Record>::iterator it;

public:
	MultiGetAddrInfo() =delete;
	MultiGetAddrInfo(MultiGetAddrInfo&&) =delete;
	explicit
	MultiGetAddrInfo( Ev::ThreadPool& threadpool_
			, std::vector<DnsSeed::Detail::Record> records_
			) : threadpool(threadpool_)
			  , records(std::move(records_))
			  {
		it = records.begin();
	}

	Ev::Io<void> run() {
		return Ev::yield().then([this]() {
			if (it == records.end())
				return Ev::lift();
			return threadpool.background<std::string>([this]() {
				return resolve(it->hostname, it->port);
			}).then([this](std::string new_hostname) {
				it->hostname = new_hostname;
				++it;
				return run();
			});
		});
	}

	std::vector<std::string> result() const {
		auto rv = std::vector<std::string>();
		for (auto const& record : records) {
			auto const& hostname = record.hostname;
			auto full_hostname = std::string();
			if (std::find( hostname.begin(), hostname.end()
				     , ':'
				     ) != hostname.end())
				full_hostname = std::string("[")
					      + hostname
					      + "]"
					      ;
			else
				full_hostname = hostname;
			rv.push_back(
				record.nodeid + "@" + full_hostname + ":" +
				enstring(record.port)
			);
		}
		return rv;
	}
};

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
				    , Ev::ThreadPool& threadpool
				    ) {
	/* The @1.0.0.1 means we use the CloudFlare resolver to
	 * get DNS SRV queries.
	 * Some ISPs have default resolvers which do not properly
	 * handle SRV queries.
	 * It seems in practice using @1.0.0.1 helps in most such
	 * cases.
	 */
	return Ev::runcmd( "dig", {"@1.0.0.1", seed, "SRV"}
			 ).then([&threadpool](std::string res) {
		auto records = Detail::parse_dig_srv(std::move(res));
		auto resolver = std::make_shared<MultiGetAddrInfo>(
			threadpool, std::move(records)
		);

		return resolver->run().then([resolver]() {
			return Ev::lift(resolver->result());
		});
	});
}

}
