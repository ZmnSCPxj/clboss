#ifndef BOSS_MSG_REQRESPTRAITS_HPP
#define BOSS_MSG_REQRESPTRAITS_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::ReqRespTraits
 *
 * @brief A traits class, **NOT** an actual message!
 *
 * @desc This describes how to set or get the `requester`
 * field of a `Request` or `Response` message.
 * If you have a `void* requester` this Just Works, but
 * you can specialize this trait if you want to use a
 * different field name for the requester.
 */
template<typename Message>
struct ReqRespTraits {
	static void set_requester(Message& m, void* requester) {
		m.requester = requester;
	}
	static void* get_requester(Message const& m) {
		return m.requester;
	}
};

}}

#endif /* !defined(BOSS_MSG_REQRESPTRAITS_HPP) */
