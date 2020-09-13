#ifndef BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_GUMSHOE_HPP
#define BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_GUMSHOE_HPP

#include<functional>
#include<memory>

namespace Ev { template<typename a> class Io; }
namespace Ln { class NodeId; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {

/** class Boss::Mod::ChannelCandidateInvestigator::Gumshoe
 *
 * @brief actual submodule that investigates the
 * candidates for uptime.
 *
 * @desc "gumshoe" is slang for "detective".
 */
class Gumshoe {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Gumshoe() =delete;

	Gumshoe(S::Bus&);
	~Gumshoe();

	/** Gumshoe::set_report_func
	 *
	 * @brief tells the gumshoe what function to use to
	 * report whether a candidate passed or failed.
	 *
	 * @desc This function must be called before the
	 * `investigate` function.
	 * The argument must not be `nullptr`.
	 *
	 * For the avoidance of doubt, the `bool` is `true`
	 * if success, `false` if failure.
	 */
	void set_report_func(std::function< Ev::Io<void>( Ln::NodeId
							, bool
							)
					  > report);

	/** Gumshoe::investigate
	 *
	 * @brief triggers investigation on whether the given
	 * node is online.
	 */
	Ev::Io<void> investigate(Ln::NodeId);
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_GUMSHOE_HPP) */
