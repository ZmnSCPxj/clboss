#include"Boltz/Service.hpp"
#include"Boss/Mod/BoltzSwapper/ServiceModule.hpp"
#include"Boss/Msg/AcceptSwapQuotation.hpp"
#include"Boss/Msg/Block.hpp"
#include"Boss/Msg/ProvideSwapQuotation.hpp"
#include"Boss/Msg/SolicitSwapQuotation.hpp"
#include"Boss/Msg/SwapCreation.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"

namespace Boss { namespace Mod { namespace BoltzSwapper {

class ServiceModule::Impl {
private:
	S::Bus& bus;
	std::unique_ptr<Boltz::Service> service;

	std::unique_ptr<std::uint32_t> blockheight;

	void start() {
		bus.subscribe<Msg::Block
			     >([this](Msg::Block const& b) {
			if (!blockheight)
				blockheight = Util::make_unique<std::uint32_t
							       >();
			*blockheight = b.height;
			return service->on_block(*blockheight);
		});

		bus.subscribe<Msg::SolicitSwapQuotation
			     >([this](Msg::SolicitSwapQuotation const& q) {
			if (!blockheight)
				return Ev::lift();

			auto solicitor = q.solicitor;
			auto amt = q.offchain_amount;

			return Ev::lift().then([this, amt]() {
				return service->get_quotation(amt);
			}).then([ this
				, solicitor
				](std::unique_ptr<Ln::Amount> fee) {
				if (!fee)
					return Ev::lift();

				return bus.raise(Msg::ProvideSwapQuotation{
					*fee, solicitor, this
				});
			});
		});

		bus.subscribe<Msg::AcceptSwapQuotation
			     >([this](Msg::AcceptSwapQuotation const& q) {
			/* If not talking to us, ignore.  */
			if (q.provider != this)
				return Ev::lift();

			if (!blockheight) {
				/* Should not happen.  Just fail it.  */
				return bus.raise(Msg::SwapCreation{
					false, "", 0, q.solicitor, this
				});
			}

			auto amt = q.offchain_amount;
			auto addr = q.onchain_address;
			auto solicitor = q.solicitor;
			return Ev::lift().then([this, amt, addr]() {
				return service->swap(amt, addr, *blockheight);
			}).then([this, solicitor
				](std::pair<std::string, std::uint32_t> res) {
				auto& invoice = res.first;
				auto timeout = res.second;
				auto success = (invoice != "");

				return bus.raise(Msg::SwapCreation{
					success,
					std::move(invoice), timeout,
					solicitor, this
				});
			});
		});
	}

public:
	Impl() =delete;
	explicit
	Impl( S::Bus& bus_
	    , std::unique_ptr<Boltz::Service> service_
	    ) : bus(bus_)
	      , service(std::move(service_))
	      { start(); }
};

ServiceModule::ServiceModule(ServiceModule&&) =default;
ServiceModule& ServiceModule::operator=(ServiceModule&&) =default;
ServiceModule::~ServiceModule() =default;

ServiceModule::ServiceModule
	( S::Bus& bus
	, std::unique_ptr<Boltz::Service> service
	) : pimpl(Util::make_unique<Impl>( bus
					 , std::move(service)
					 ))
	  { }

}}}
