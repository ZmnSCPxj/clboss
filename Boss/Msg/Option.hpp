#ifndef BOSS_MSG_OPTION_HPP
#define BOSS_MSG_OPTION_HPP

#include"Jsmn/Object.hpp"
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::Option
 *
 * @brief emitted during `init` handling, providing the value
 * of an option that we registered.
 */
struct Option {
	std::string name;
	Jsmn::Object value;
};

}}

#endif /* !defined(BOSS_MSG_OPTION_HPP) */
