#ifndef BOSS_MSG_MANIFESTHOOK_HPP
#define BOSS_MSG_MANIFESTHOOK_HPP

#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ManifestHook
 *
 * @brief emitted in respone to Boss::Msg::Manifestation
 * to register a hook.
 * Multiple modules can register the same hook and listen
 * for Boss::Msg::CommandRequest for that hook, though
 * only the first one that responds will drive the hook.
 */
struct ManifestHook {
	std::string name;
};

}}

#endif /* !defined(BOSS_MSG_MANIFESTHOOK_HPP) */
