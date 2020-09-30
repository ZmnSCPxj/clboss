#ifndef BOSS_MOD_NODEBALANCESWAPPER_HPP
#define BOSS_MOD_NODEBALANCESWAPPER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::NodeBalanceSwapper
 *
 * @brief swaps offchain amounts for
 * onchain amounts if our offchain
 * incoming capacity is low.
 *
 * @desc this handles node-level balancing,
 * i.e. we should have both incoming and
 * outgoing capacity.
 * Other modules are responsible for
 * channel-level balancing.
 */
class NodeBalanceSwapper {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	NodeBalanceSwapper() =delete;

	NodeBalanceSwapper(NodeBalanceSwapper&&);
	NodeBalanceSwapper& operator=(NodeBalanceSwapper&&);
	~NodeBalanceSwapper();

	explicit
	NodeBalanceSwapper(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_NODEBALANCESWAPPER_HPP) */
