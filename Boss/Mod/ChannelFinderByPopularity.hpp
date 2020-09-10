#ifndef BOSS_MOD_CHANNELFINDERBYPOPULARITY_HPP
#define BOSS_MOD_CHANNELFINDERBYPOPULARITY_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ChannelFinderByPopularity
 *
 * @brief module to find channel candidates by popularity.
 *
 * @desc we do not, in fact, propose the popular candidates.
 * Instead, after locating some popular nodes, we select
 * one of *its* peers as the proposal, with the popular
 * node as the patron.
 */
class ChannelFinderByPopularity {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ChannelFinderByPopularity() =delete;
	ChannelFinderByPopularity(ChannelFinderByPopularity const&) =delete;

	ChannelFinderByPopularity(S::Bus&);
	ChannelFinderByPopularity(ChannelFinderByPopularity&&);
	~ChannelFinderByPopularity();
};

}}

#endif /* BOSS_MOD_CHANNELFINDERBYPOPULARITY_HPP */
