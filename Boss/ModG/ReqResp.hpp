#ifndef BOSS_MODG_REQRESP_HPP
#define BOSS_MODG_REQRESP_HPP

#include"Boss/ModG/Detail/ReqRespBase.hpp"
#include"Boss/Msg/ReqRespTraits.hpp"
#include"Boss/Shutdown.hpp"
#include"Ev/Io.hpp"
#include"S/Bus.hpp"
#include<memory>

namespace Boss { namespace ModG {

/** class Boss::ModG::ReqResp<Request, Response>
 *
 * @brief provides a blocking operation that makes a
 * `Request` and blocks until it receives a corresponding
 * `Response`.
 *
 * @desc the `Request`-`Response` pair should have an
 * "identifier" pointer in the message, such that the
 * `Request` includes this pointer, which is copied to
 * the corresponding `Response`.
 */
template< typename Request
	, typename Response
	>
class ReqResp {
private:
	S::Bus& bus;

	std::unique_ptr<Detail::ReqRespBase> base;

public:
	ReqResp() =delete;
	explicit
	ReqResp( S::Bus& bus_
	       ) : bus(bus_)
		 {
		bus.subscribe<Response>([this](Response const& resp) {
			if (!base)
				return Ev::lift();
			auto presp = std::make_shared<Response>(resp);
			auto w_get_annotation = [](std::shared_ptr<void
								  > const& p) {
				return Msg::ReqRespTraits<Response>::get_requester(
					*reinterpret_cast<Response*>(p.get())
				);
			};
			return base->on_response( std::move(presp)
						, std::move(w_get_annotation)
						);
		});
		bus.subscribe<Shutdown>([this](Shutdown const& _) {
			base = nullptr;
			return Ev::lift();
		});
	}

	Ev::Io<Response> execute(Request req) {
		auto preq = std::make_shared<Request>(req);
		if (!base)
			base = Util::make_unique<Detail::ReqRespBase>();
		auto w_set_annotation = []( std::shared_ptr<void> const& p
					  , void* key
					  ){
			Msg::ReqRespTraits<Request>::set_requester
				( *reinterpret_cast<Request*>(p.get())
				, key
				);
		};
		auto w_broadcast = [this](std::shared_ptr<void> p) {
			auto& msg = *reinterpret_cast<Request*>(p.get());
			return bus.raise(std::move(msg));
		};
		return base->execute( std::move(preq)
				    , w_set_annotation
				    , w_broadcast
				    ).then([](std::shared_ptr<void> vresp) {
			return Ev::lift(std::move(
				*reinterpret_cast<Response*>(vresp.get())
			));
		});
	}
};

}}

#endif /* !defined(BOSS_MODG_REQRESP_HPP) */
