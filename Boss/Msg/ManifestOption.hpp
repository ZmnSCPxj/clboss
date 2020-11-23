#ifndef BOSS_MSG_MANIFESTOPTION_HPP
#define BOSS_MSG_MANIFESTOPTION_HPP

#include"Boss/Msg/OptionType.hpp"
#include"Json/Out.hpp"
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ManifestOption
 *
 * @brief emit in response to `Boss::Msg::Manifestation` to
 * register an option.
 */
struct ManifestOption {
	std::string name;
	OptionType type;
	Json::Out default_value;
	std::string description;
};

}}

#endif /* !defined(BOSS_MSG_MANIFESTOPTION_HPP) */
