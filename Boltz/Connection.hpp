#ifndef BOLTZ_CONNECTION_HPP
#define BOLTZ_CONNECTION_HPP

#include<memory>
#include<stdexcept>
#include<string>

namespace Ev { template<typename a> class Io; }
namespace Ev { class ThreadPool; }
namespace Jsmn { class Object; }
namespace Json { class Out; }

namespace Boltz {

/** class Boltz::Connection
 *
 * @brief handles a connection to some Boltz
 * server, and wrangles JSON comunications
 * with it.
 */
class Connection {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Connection() =delete;
	Connection(Connection const&) =delete;

	Connection(Connection&&);
	~Connection();
	explicit
	Connection( Ev::ThreadPool& threadpool
		  /* Base address of the API endpoint.  */
		  , std::string api_base = "https://boltz.exchange/api"
		  /* SOCKS5 proxy to use.  Empty string means no proxy.  */
		  , std::string proxy = ""
		  );

	/* Send an API request.  */
	Ev::Io<Jsmn::Object>
	api( std::string api /* e.g. "/createswap" */
	   /* nullptr if GET, or the request body if POST.  */
	   , std::unique_ptr<Json::Out> params
	   );
};

/** Boltz::ApiError
 *
 * @brief thrown when communications with
 * the BOLTZ server has problems.
 */
class ApiError : public std::runtime_error {
public:
	ApiError(std::string const& e
		) : std::runtime_error(e) { }
};

}

#endif /* !defined(BOLTZ_CONNECTION_HPP) */
