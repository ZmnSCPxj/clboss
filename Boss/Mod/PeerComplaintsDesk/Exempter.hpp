#ifndef BOSS_MOD_PEERCOMPLAINTSDESK_EXEMPTER_HPP
#define BOSS_MOD_PEERCOMPLAINTSDESK_EXEMPTER_HPP

#include<map>
#include<memory>
#include<string>

namespace Boss { namespace Mod { namespace PeerComplaintsDesk {
	class Unmanager;
}}}
namespace Ev { template<typename a> class Io; }
namespace Ln { class NodeId; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace PeerComplaintsDesk {

/** class Boss::Mod::PeerComplaintsDesk::Exempter
 *
 * @brief Provides exemptions from complaints, causing the
 * complaints desk to ignore complaints about certain peers.
 */
class Exempter {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Exempter() =delete;
	Exempter(Exempter const&) =delete;

	Exempter(Exempter&&);
	~Exempter();

	explicit
	Exempter(S::Bus& bus, Unmanager& unmanager);

	/** Boss::Mod::PeerComplaintsDesk::Exempter::get_exemptions
	 *
	 * @brief Creates a map containing reasons to ignore
	 * the complaints about certain peers.
	 */
	Ev::Io<std::map<Ln::NodeId, std::string>>
	get_exemptions();
};

}}}

#endif /* !defined(BOSS_MOD_PEERCOMPLAINTSDESK_EXEMPTER_HPP) */
