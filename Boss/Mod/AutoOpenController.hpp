#ifndef BOSS_MOD_AUTOOPENCONTROLLER_HPP
#define BOSS_MOD_AUTOOPENCONTROLLER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::AutoOpenController
 *
 * @brief Provides RPC commands `clboss-auto-open-channels` and
 * `clboss-temporarily-auto-open-channels` to tell CLBOSS whether or not to
 * automatically open channels.
 * Also exposes the state to other modules via the
 * `Msg::RequestGetAutoOpenFlag` message.
 */
class AutoOpenController {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	AutoOpenController() =delete;
	AutoOpenController(AutoOpenController const&) =delete;

	AutoOpenController(AutoOpenController&&);
	~AutoOpenController();

	explicit
	AutoOpenController(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_AUTOOPENCONTROLLER_HPP) */
