#ifndef BOSS_MSG_OPTIONTYPE_HPP
#define BOSS_MSG_OPTIONTYPE_HPP

namespace Boss { namespace Msg {

/** enum Boss::Msg::OptionType
 *
 * @brief the types that a `lightningd` option can have.
 */
enum OptionType {
	OptionType_String,
	OptionType_Bool,
	OptionType_Int,
	OptionType_Flag
};

}}

#endif /* !defined(BOSS_MSG_OPTIONTYPE_HPP) */
