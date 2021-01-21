#ifndef BOLTZ_DETAIL_FALLBACKCONNECTION_HPP
#define BOLTZ_DETAIL_FALLBACKCONNECTION_HPP

#include"Boltz/ConnectionIF.hpp"
#include<memory>
#include<utility>

namespace Boltz { namespace Detail {

/** class Boltz::Detail::FallbackConnection
 *
 * @brief a connection that tries one sub-connection first,
 * then the other sub-connection if the first fails.
 */
class FallbackConnection : public Boltz::ConnectionIF {
private:
	std::unique_ptr<Boltz::ConnectionIF> first;
	std::unique_ptr<Boltz::ConnectionIF> second;

public:
	FallbackConnection() =delete;

	FallbackConnection(FallbackConnection&&) =default;

	explicit
	FallbackConnection( std::unique_ptr<Boltz::ConnectionIF> first_
			  , std::unique_ptr<Boltz::ConnectionIF> second_
			  ) : first(std::move(first_))
			    , second(std::move(second_))
			    { }

	/* Send an API request.  */
	Ev::Io<Jsmn::Object>
	api( std::string api /* e.g. "/createswap" */
	   /* nullptr if GET, or the request body if POST.  */
	   , std::unique_ptr<Json::Out> params
	   ) override;
};

}}

#endif /* !defined(BOLTZ_DETAIL_FALLBACKCONNECTION_HPP) */
