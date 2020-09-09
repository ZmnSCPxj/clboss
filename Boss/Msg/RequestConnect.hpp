#ifndef BOSS_MSG_REQUESTCONNECT_HPP
#define BOSS_MSG_REQUESTCONNECT_HPP

#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestConnect
 *
 * @brief emitted to ask the Connector module
 * to connect a specific node, and then respond
 * with whether the connection succeeded or not.
 */
struct RequestConnect {
	/* Can be just nodeid, or else nodeid@host:port, or basically
	 * anything the `connect` command understands.  */
	std::string node;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTCONNECT_HPP) */
