#ifndef BOSS_MOD_LISTPEERSANALYZER_HPP
#define BOSS_MOD_LISTPEERSANALYZER_HPP

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ListpeersAnalyzer
 *
 * @brief converts `Boss::Msg::ListpeersResult` to
 * `Boss::Msg::ListpeersAnalyzedResult`.
 */
class ListpeersAnalyzer {
public:
	ListpeersAnalyzer(S::Bus&);
};

}}

#endif /* !defined(BOSS_MOD_LISTPEERSANALYZER_HPP) */
