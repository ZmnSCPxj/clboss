#ifndef BOSS_MOD_AMOUNTSETTINGSHANDLER_HPP
#define BOSS_MOD_AMOUNTSETTINGSHANDLER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

class AmountSettingsHandler {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	AmountSettingsHandler() =delete;
	AmountSettingsHandler(AmountSettingsHandler const&) =delete;

	AmountSettingsHandler(AmountSettingsHandler&&);

	explicit
	AmountSettingsHandler(S::Bus&);
};

}}

#endif /* !defined(BOSS_MOD_AMOUNTSETTINGSHANDLER_HPP) */
