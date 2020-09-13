#ifndef BOSS_MOD_CHANNELCANDIDATEPREINVESTIGATOR_HPP
#define BOSS_MOD_CHANNELCANDIDATEPREINVESTIGATOR_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

class ChannelCandidatePreinvestigator {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ChannelCandidatePreinvestigator() =delete;

	ChannelCandidatePreinvestigator(S::Bus&);
	ChannelCandidatePreinvestigator(ChannelCandidatePreinvestigator&&);
	~ChannelCandidatePreinvestigator();
};

}}

#endif /* !defined(BOSS_MOD_CHANNELCANDIDATEPREINVESTIGATOR_HPP) */
