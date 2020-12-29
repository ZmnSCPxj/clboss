#include"Boss/Mod/ChannelCreator/Reprioritizer.hpp"
#include"Ev/Io.hpp"
#include"Ev/foreach.hpp"
#include"Ev/yield.hpp"
#include"Ln/NodeId.hpp"
#include"Net/IPAddr.hpp"
#include"Net/IPAddrOrOnion.hpp"
#include"Net/IPBinnerIF.hpp"
#include"Net/get_bin_of_onion.hpp"
#include"Secp256k1/SignerIF.hpp"
#include"Sha256/Hash.hpp"
#include<map>

namespace Boss { namespace Mod { namespace ChannelCreator {

class Reprioritizer::Run {
private:
	Secp256k1::SignerIF& signer;
	Net::IPBinnerIF const& binner;
	std::function< Ev::Io<std::unique_ptr<Net::IPAddrOrOnion>>(Ln::NodeId)
		     > const& get_node_addr;
	std::function< Ev::Io<std::vector<Ln::NodeId>>()> const& get_peers;

	std::vector<std::pair<Ln::NodeId, Ln::NodeId>> orig_proposals;
	std::vector<std::pair<Ln::NodeId, Ln::NodeId>> final_proposals;

public:
	Run() =delete;

	Run( Secp256k1::SignerIF& signer_
	   , Net::IPBinnerIF const& binner_
	   , std::function< Ev::Io<std::unique_ptr<Net::IPAddrOrOnion>>(Ln::NodeId)
			   > const& get_node_addr_
	   , std::function< Ev::Io<std::vector<Ln::NodeId>>()> const& get_peers_
	   , std::vector<std::pair<Ln::NodeId, Ln::NodeId>> proposals
	   ) : signer(signer_)
	     , binner(binner_)
	     , get_node_addr(get_node_addr_)
	     , get_peers(get_peers_)
	     , orig_proposals(std::move(proposals))
	     { }

	static
	Ev::Io<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
	run(std::shared_ptr<Run> self) {
		return self->core_run().then([self]() {
			return Ev::lift(std::move(self->final_proposals));
		});
	}

private:
	std::map<Net::IPBin, std::size_t> nodes_per_bin;
	std::map< std::size_t
		, std::vector<std::pair< Ln::NodeId
				       , Ln::NodeId>>> nodes_per_binsize;
	std::vector<std::pair<Ln::NodeId, Ln::NodeId>>::iterator proposal_it;

	Ev::Io<Net::IPBin> get_bin_of(Ln::NodeId n) {
		return get_node_addr(std::move(n)
				    ).then([this](std::unique_ptr<Net::IPAddrOrOnion> paddr) {
			if (!paddr)
				return Ev::lift(Net::IPBin(0));
			auto const& addr = *paddr;
			if (addr.is_onion()) {
				auto tweak_hash = [this](std::uint8_t data[32]) {
					auto sha256 = signer.get_privkey_salted_hash(data);
					sha256.to_buffer(data);
				};

				auto onion = std::string();
				addr.to_onion(onion);

				auto rv = Net::get_bin_of_onion( tweak_hash
							       , onion
							       );
				return Ev::lift(rv);
			} else {
				auto ip = Net::IPAddr();
				addr.to_ip_addr(ip);
				auto rv = binner.get_bin_of(ip);
				return Ev::lift(rv);
			}
		});
	}

	Ev::Io<void> core_run() {
		return Ev::lift().then([this]() {

			/* Initialize nodes-per-bin.  */
			return get_peers();
		}).then([this](std::vector<Ln::NodeId> current_peers) {
			auto update_bin = [this](Ln::NodeId n) {
				return get_bin_of(n).then([this](Net::IPBin bin) {
					auto it = nodes_per_bin.find(bin);
					if (it == nodes_per_bin.end())
						nodes_per_bin[bin] = 1;
					else
						++it->second;
					return Ev::lift();
				});
			};

			return Ev::foreach( std::move(update_bin)
					  , std::move(current_peers)
					  );
		}).then([this]() {

			/* Start the reprioritize loop.
			 * We cannot use Ev::foreach since that executes each
			 * item in parallel; we need explicitly sequential
			 * processing of the vector.
			 */
			proposal_it = orig_proposals.begin();
			return reprioritize_loop();
		});
	}
	Ev::Io<void> reprioritize_loop() {
		return Ev::yield().then([this]() {
			if (proposal_it == orig_proposals.end())
				return build_final_proposals();
			return reprioritize_step().then([this]() {
				++proposal_it;
				return reprioritize_loop();
			});
		});
	}
	Ev::Io<void> reprioritize_step() {
		return Ev::lift().then([this]() {
			/* Find bin of proposal.  */
			return get_bin_of(proposal_it->first);
		}).then([this](Net::IPBin bin) {
			/* Check current number of nodes in that bin, and update.  */
			auto it = nodes_per_bin.find(bin);
			auto binsize = std::size_t();
			if (it == nodes_per_bin.end()) {
				binsize = 0;
				nodes_per_bin[bin] = 1;
			} else {
				binsize = it->second;
				++it->second;
			}

			/* Now that we know the number of nodes in that bin, we can
			 * put the proposal in the correct bin size.
			 */
			auto it2 = nodes_per_binsize.find(binsize);
			if (it2 == nodes_per_binsize.end())
				nodes_per_binsize[binsize].push_back(std::move(*proposal_it));
			else
				it2->second.push_back(std::move(*proposal_it));

			return Ev::lift();
		});
	}
	Ev::Io<void> build_final_proposals() {
		return Ev::lift().then([this]() {
			/* We are dependent on the fact that std::map is ordered!  */
			for (auto& binsizes : nodes_per_binsize) {
				final_proposals.insert( final_proposals.end()
						      , binsizes.second.begin()
						      , binsizes.second.end()
						      );
			}
			return Ev::lift();
		});
	}
};

Reprioritizer::~Reprioritizer() =default;

Reprioritizer::Reprioritizer( Secp256k1::SignerIF& signer_
			    , std::unique_ptr<Net::IPBinnerIF> binner_
			    , std::function< Ev::Io<std::unique_ptr<Net::IPAddrOrOnion>
						   >(Ln::NodeId)
					   > get_node_addr_
			    , std::function< Ev::Io<std::vector<Ln::NodeId>>()> get_peers_
			    ) : signer(signer_)
			      , binner(std::move(binner_))
			      , get_node_addr(std::move(get_node_addr_))
			      , get_peers(std::move(get_peers_))
			      { }
Ev::Io<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
Reprioritizer::reprioritize(std::vector<std::pair<Ln::NodeId, Ln::NodeId>> proposals) {
	auto run = std::make_shared<Run>( signer, *binner, get_node_addr, get_peers
					, std::move(proposals)
					);
	return Run::run(run);
}

}}}
