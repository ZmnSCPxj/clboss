#ifndef BOSS_MOD_PEERFROMSCIDMAPPER_HPP
#define BOSS_MOD_PEERFROMSCIDMAPPER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::PeerFromScidMapper
 *
 * @brief Handles `Boss::Msg::RequestePeerFromScid` messages,
 * figuring out the peer node ID from a given SCID, and
 * broadcasts `Boss::Msg::ResponsePeerFromScid` in response.
 */
class PeerFromScidMapper {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	PeerFromScidMapper() =delete;
	PeerFromScidMapper(PeerFromScidMapper const&) =delete;

	PeerFromScidMapper(PeerFromScidMapper&&);
	~PeerFromScidMapper();

	explicit
	PeerFromScidMapper(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_PEERFROMSCIDMAPPER_HPP) */
