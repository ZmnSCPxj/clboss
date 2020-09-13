#ifndef BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_MAIN_HPP
#define BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_MAIN_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {

class Gumshoe;
class Secretary;
class Manager;

class Main {
private:
	std::unique_ptr<Secretary> secretary;
	std::unique_ptr<Gumshoe> gumshoe;
	std::unique_ptr<Manager> manager;

public:
	Main() =delete;
	Main(Main&&);
	~Main();

	explicit
	Main(S::Bus& bus);
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_MAIN_HPP) */
