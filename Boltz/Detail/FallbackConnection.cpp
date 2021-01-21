#include"Boltz/Detail/FallbackConnection.hpp"
#include"Ev/Io.hpp"
#include"Json/Out.hpp"
#include"Util/make_unique.hpp"
#include<assert.h>

namespace {

std::unique_ptr<Json::Out>
copy_params(std::shared_ptr<Json::Out> const& shared_params) {
	if (!shared_params)
		return nullptr;

	return Util::make_unique<Json::Out>(*shared_params);
}

}

namespace Boltz { namespace Detail {

Ev::Io<Jsmn::Object>
FallbackConnection::api( std::string api /* e.g. "/createswap" */
		       /* nullptr if GET, or the request body if POST.  */
		       , std::unique_ptr<Json::Out> params
		       ) {
	assert(first && second);

	auto save_params = std::shared_ptr<Json::Out>(std::move(params));
	return Ev::lift().then([api, save_params, this]() {
		return first->api(api, copy_params(save_params));
	}).catching<Boltz::ApiError>([api, save_params, this](Boltz::ApiError e) {
		return second->api(api, copy_params(save_params));
	});
}

}}
