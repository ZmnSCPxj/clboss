#ifndef BOLTZ_DETAIL_NULLCONNECTION_HPP
#define BOLTZ_DETAIL_NULLCONNECTION_HPP

#include"Boltz/ConnectionIF.hpp"

namespace Boltz { namespace Detail {

/** class Boltz::Detail::NullConnection
 *
 * @brief a connection to a Boltz-like service that
 * always fails.
 */
class NullConnection : public Boltz::ConnectionIF {
public:
	Ev::Io<Jsmn::Object>
	api( std::string api /* e.g. "/createswap" */
	   /* nullptr if GET, or the request body if POST.  */
	   , std::unique_ptr<Json::Out> params
	   ) override;
};

}}

#endif /* !defined(BOLTZ_DETAIL_NULLCONNECTION_HPP) */
