#include"Boss/Mod/NodeBalanceSwapper.hpp"
#include"Boss/ModG/Swapper.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Boss/Msg/OnchainFee.hpp"
#include"Boss/log.hpp"
#include"Jsmn/Object.hpp"
#include"Ln/Amount.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<sstream>

namespace {

/* The minimum percentage of total channel capacity that is
 * incoming.
 *
 * Cannot be higher than 50.
 */
auto const min_receivable_percent = double(35);
/* If fees are high, we do not swap, ***unless*** our incoming
 * capacity is below this percentage, in which case we end up
 * triggering anyway.
 */
auto const min_receivable_percent_highfees = double(2.5);

}

namespace Boss { namespace Mod {

class NodeBalanceSwapper::Impl : ModG::Swapper {
private:
	std::unique_ptr<bool> fees_low;

	void start() {
		bus.subscribe<Msg::OnchainFee
			     >([this](Msg::OnchainFee const& m) {
			if (!fees_low)
				fees_low = Util::make_unique<bool>();
			*fees_low = m.fees_low;
			return Ev::lift();
		});
		bus.subscribe<Msg::ListpeersResult
			     >([this](Msg::ListpeersResult const& m) {
			if (!fees_low)
				/* Pretend fees are high temporarily.  */
				fees_low = Util::make_unique<bool>(false);
			/* Analyze the result.  */
			auto total_recv = Ln::Amount::sat(0);
			auto total_send = Ln::Amount::sat(0);
			try {
				for ( auto i = std::size_t(0)
				    ; i < m.peers.size()
				    ; ++i
				    ) {
					auto peer = m.peers[i];
					auto channels = peer["channels"];
					for ( auto j = std::size_t(0)
					    ; j < channels.size()
					    ; ++j
					    ) {
						auto chan = channels[j];
						/* Skip non-active channels.
						 */
						if ( std::string(chan["state"])
						  != "CHANNELD_NORMAL"
						   )
							continue;
						/* Skip unpublished channels,
						 * they cannot be used for
						 * forwarding anyway.
						 */
						if (chan["private"])
							continue;
						auto recv = std::string(chan[
							"receivable_msat"
						]);
						auto send = std::string(chan[
							"spendable_msat"
						]);
						total_recv += Ln::Amount(recv);
						total_send += Ln::Amount(send);
					}
				}
			} catch (Jsmn::TypeError const&) {
				/* Should never happen.... */
				auto os = std::ostringstream();
				os << m.peers;
				return Boss::log( bus, Error
						, "NodeBalanceSwapper: "
						  "Unexpected listpeers: %s"
						, os.str().c_str()
						);
			}

			/* Avoid division by 0. */
			if ( total_recv == Ln::Amount::sat(0)
			  && total_send == Ln::Amount::sat(0)
			   )
				return Ev::lift();

			auto total = total_recv + total_send;
			auto recv_ratio = total_recv / total;
			auto recv_percent = recv_ratio * 100;
			if (recv_percent >= min_receivable_percent)
				/* Nothing to do.  */
				return Ev::lift();

			auto f = [ this
				 , total_recv
				 , total_send
				 , recv_percent
				 ]( Ln::Amount& amount
				  , std::string& why
				  ) {
				amount = ( (total_send - total_recv)
					 * 2
					 ) / 3;

				auto os = std::ostringstream();
				os << "receivable = " << total_recv << ", "
				   << "spendable = " << total_send << "; "
				   << "%receivable = " << recv_percent << "; "
				    ;
				if (*fees_low) {
					os << "sending funds onchain";
					why = os.str();
					return true;
				} else if ( recv_percent
					  > min_receivable_percent_highfees
					  ) {
					os << "but fees are high";
					why = os.str();
					return false;
				} else {
					os << "fees are high but we have "
					   << "too little receivable"
					   ;
					why = os.str();
					return false;
				}
			};

			return trigger_swap(f);
		});
	}

public:
	explicit
	Impl(S::Bus& bus_) : ModG::Swapper( bus_
					  , "NodeBalanceSwapper"
					  , "incoming_capacity_swapper"
					  ) { start(); }
};

NodeBalanceSwapper::NodeBalanceSwapper(NodeBalanceSwapper&&) =default;
NodeBalanceSwapper& NodeBalanceSwapper::operator=(NodeBalanceSwapper&&)
	=default;
NodeBalanceSwapper::~NodeBalanceSwapper() =default;

NodeBalanceSwapper::NodeBalanceSwapper(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
