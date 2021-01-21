#include"Boltz/Detail/NullConnection.hpp"
#include"Ev/Io.hpp"

namespace Boltz { namespace Detail {

Ev::Io<Jsmn::Object>
NullConnection::api( std::string api /* e.g. "/createswap" */
		   /* nullptr if GET, or the request body if POST.  */
		   , std::unique_ptr<Json::Out> params
		   ) {
	return Ev::lift().then([]() -> Ev::Io<Jsmn::Object> {
		throw Boltz::ApiError("no connection available");
	});
}

}}
