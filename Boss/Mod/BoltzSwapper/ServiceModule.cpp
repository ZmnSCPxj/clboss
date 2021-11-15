#include"Boltz/Service.hpp"
#include"Boltz/SwapInfo.hpp"
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
	std::string label;

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
					*fee, solicitor, this,
					label
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
					false,
					"", Sha256::Hash(), 0,
					q.solicitor, this
				});
			}

			auto amt = q.offchain_amount;
			auto addr = q.onchain_address;
			auto solicitor = q.solicitor;
			return Ev::lift().then([this, amt, addr]() {
				return service->swap(amt, addr, *blockheight);
			}).then([this, solicitor
				](Boltz::SwapInfo res) {
				auto& invoice = res.invoice;
				auto timeout = res.timeout;
				auto& hash = res.hash;
				auto success = (invoice != "");

				return bus.raise(Msg::SwapCreation{
					success,
					std::move(invoice),
					std::move(hash),
					timeout,
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
	    , std::string label_
	    ) : bus(bus_)
	      , service(std::move(service_))
	      , label(std::string("Boltz::Service(\"") + label_ + "\")")
	      { start(); }
};

ServiceModule::ServiceModule(ServiceModule&&) =default;
ServiceModule& ServiceModule::operator=(ServiceModule&&) =default;
ServiceModule::~ServiceModule() =default;

ServiceModule::ServiceModule
	( S::Bus& bus
	, std::unique_ptr<Boltz::Service> service
	, std::string label
	) : pimpl(Util::make_unique<Impl>( bus
					 , std::move(service)
					 , std::move(label)
					 ))
	  { }

}}}
