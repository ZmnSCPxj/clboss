#ifndef BOLTZ_ENVIF_HPP
#define BOLTZ_ENVIF_HPP

#include<cstdint>
#include<string>

namespace Bitcoin { class Tx; }
namespace Ev { template<typename a> class Io; }

namespace Boltz {

/** class Boltz::EnvIF
 *
 * @brief abstract interface to the local
 * environment the BOLTZ-swapping code is
 * running in.
 */
class EnvIF {
public:
	/** Boltz::EnvIF::get_feerate
	 *
	 * @brief get the recommended feerate
	 * for normal transactions.
	 */
	virtual
	Ev::Io<std::uint32_t> get_feerate() =0;

	/** Boltz::EnvIF::broadcast_tx
	 *
	 * @brief broadcasts the transaction.
	 *
	 * @return false if broadcasting failed,
	 * true if broadcasting succeeded.
	 */
	virtual
	Ev::Io<bool> broadcast_tx(Bitcoin::Tx) =0;

	/** Boltz::EnvIF::logd
	 *
	 * @brief prints a Debug-level log.
	 */
	virtual
	Ev::Io<void> logd(std::string) =0;

	/** Boltz::EnvIF::loge
	 *
	 * @brief prints an Erro-level log.
	 */
	virtual
	Ev::Io<void> loge(std::string) =0;
};

}

#endif /* !defined(BOLTZ_ENVIF_HPP) */
