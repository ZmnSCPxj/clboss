#ifndef BOSS_MSG_PROVIDESTATUS
#define BOSS_MSG_PROVIDESTATUS

#include"Json/Out.hpp"
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ProvideStatus
 *
 * @brief emitted by modules in response to
 * `Boss::Msg::SolicitStatus` in order to
 * report their status.
 */
struct ProvideStatus {
	/* Field name to use in the status response.  */
	std::string key;
	/* Content of the field.  */
	Json::Out value;
};

}}

#endif /* !defined(BOSS_MSG_PROVIDESTATUS) */
