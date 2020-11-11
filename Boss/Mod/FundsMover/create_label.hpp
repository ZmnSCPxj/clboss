#ifndef BOSS_MOD_FUNDSMOVER_CREATE_LABEL_HPP
#define BOSS_MOD_FUNDSMOVER_CREATE_LABEL_HPP

#include<string>

namespace Sha256 { class Hash; }

namespace Boss { namespace Mod { namespace FundsMover {

/** Boss::Mod::FundsMover::create_label
 *
 * @brief Creates the label to be used for funds-movement
 * attempts.
 */
std::string create_label(Sha256::Hash const& payment_hash);

/** Boss::Mod::FundsMover::is_our_label
 *
 * @brief Determine if the given label matches one that we
 * created.
 */
bool is_our_label(std::string const&);

}}}

#endif /* !defined(BOSS_MOD_FUNDSMOVER_CREATE_LABEL_HPP) */
