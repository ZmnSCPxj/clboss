#ifndef BOLTZ_CONNECTIONIF_HPP
#define BOLTZ_CONNECTIONIF_HPP

#include"Util/BacktraceException.hpp"
#include<memory>
#include<stdexcept>
#include<string>

namespace Ev { template<typename a> class Io; }
namespace Jsmn { class Object; }
namespace Json { class Out; }

namespace Boltz {

/** class Boltz::ConnectionIF
 *
 * @brief interface to an object that provides
 * access to a Boltz API.
 */
class ConnectionIF {
public:
	virtual ~ConnectionIF() { }

	/* Send an API request.  */
	virtual
	Ev::Io<Jsmn::Object>
	api( std::string api /* e.g. "/createswap" */
	   /* nullptr if GET, or the request body if POST.  */
	   , std::unique_ptr<Json::Out> params
	   ) =0;
};


/** Boltz::ApiError
 *
 * @brief thrown when communications with
 * the BOLTZ server has problems.
 */
class ApiError : public Util::BacktraceException<std::runtime_error> {
public:
	ApiError(std::string const& e
		) : Util::BacktraceException<std::runtime_error>(e) { }
};

}

#endif /* !defined(BOLTZ_CONNECTIONIF_HPP) */
