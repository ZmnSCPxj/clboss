#include"Boss/Mod/ChannelCreator/RearrangerBySize.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include"Util/make_unique.hpp"
#include<algorithm>
#include<iterator>

namespace {

/* Group the input proposals by this many, then sort within each
 * group.
 * Note: if you change this, make sure to change also the test
 * in tests/boss/test_channelcreator_rearrangerbysize.cpp
 */
auto constexpr group_by = std::size_t(3);

}

namespace Boss { namespace Mod { namespace ChannelCreator {

class RearrangerBySize::Impl {
private:
	DowserFunc dowser_func;

	typedef std::pair<Ln::NodeId, Ln::NodeId> Proposal;
	typedef std::vector<Proposal> ProposalsVector;
	typedef std::pair<Proposal, Ln::Amount> ProposalCapacity;
	typedef std::vector<ProposalCapacity> ProposalCapacitiesVector;

public:
	explicit
	Impl( DowserFunc dowser_func_
	    ) : dowser_func(std::move(dowser_func_))
	      { }

	Ev::Io<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
	rearrange_by_size(std::vector< std::pair<Ln::NodeId, Ln::NodeId>
				     > proposals_) {
		typedef std::vector<std::pair<Ln::NodeId, Ln::NodeId>>
			ProposalsVector;
		auto proposals = std::make_shared<ProposalsVector>();
		*proposals = std::move(proposals_);

		return Ev::lift().then([this, proposals]() {
			auto func = [this]( Proposal p
					  ) {
				return dowser_func( p.first
						  , p.second
						  ).then([p](Ln::Amount a) {
					return Ev::lift(std::make_pair(p, a));
				});
			};
			return Ev::map(func, std::move(*proposals));
		}).then([](ProposalCapacitiesVector proposals) {
			for ( auto p = proposals.begin()
			    ; p < proposals.end()
			    ; p += group_by
			    ) {
				auto e = p + group_by;
				if (e > proposals.end())
					e = proposals.end();
				std::sort(p, e, []( ProposalCapacity a
						  , ProposalCapacity b
						  ) {
					return a.second > b.second;
				});
			}

			auto ret = ProposalsVector();
			std::transform( proposals.begin(), proposals.end()
				      , std::back_inserter(ret)
				      , [](ProposalCapacity pc) {
				return pc.first;
			});
			return Ev::lift(std::move(ret));
		});
	}
};

RearrangerBySize::RearrangerBySize(RearrangerBySize&&) =default;
RearrangerBySize::~RearrangerBySize() =default;

RearrangerBySize::RearrangerBySize( DowserFunc dowser_func)
	: pimpl(Util::make_unique<Impl>(std::move(dowser_func))) { }


Ev::Io<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
RearrangerBySize::rearrange_by_size(std::vector< std::pair< Ln::NodeId
							  , Ln::NodeId
							  >> proposals) {
	return pimpl->rearrange_by_size(std::move(proposals));
}

}}}
