#ifndef BOSS_MOD_FUNDSMOVER_CLAIMER_HPP
#define BOSS_MOD_FUNDSMOVER_CLAIMER_HPP

#include"Ln/Preimage.hpp"
#include"Secp256k1/Random.hpp"
#include"Sha256/Hash.hpp"
#include<unordered_map>

namespace S { class Bus; }

namespace Boss { namespace Mod { namespace FundsMover {

/** class Boss::Mod::FundsMover::Claimer
 *
 * @brief claims incoming funds at the destination.
 */
class Claimer {
private:
	S::Bus& bus;

	Secp256k1::Random rand;

	struct Entry {
		double timeout;
		Ln::Preimage preimage;
	};
	std::unordered_map<Sha256::Hash, Entry> entries;

	void start();

public:
	Claimer() =delete;
	Claimer(Claimer&&) =delete;
	Claimer(Claimer const&) =delete;

	explicit
	Claimer(S::Bus& bus_) : bus(bus_) { start(); }

	/* Generate a new preimage for a new Attempter.  */
	Ln::Preimage generate();
};

}}}

#endif /* !defined(BOSS_MOD_FUNDSMOVER_CLAIMER_HPP) */
