#ifndef BOSS_MOD_LISTFUNDSANALYZER_HPP
#define BOSS_MOD_LISTFUNDSANALYZER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ListfundsAnalyzer
 *
 * @brief Module to analyze the result of `listfunds` command.
 */
class ListfundsAnalyzer {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ListfundsAnalyzer() =delete;
	ListfundsAnalyzer(ListfundsAnalyzer&&);
	~ListfundsAnalyzer();
	explicit
	ListfundsAnalyzer(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_LISTFUNDSANALYZER_HPP) */
