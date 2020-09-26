#ifndef BOSS_MOD_BOLTZSWAPPER_SERVICEMODULE_HPP
#define BOSS_MOD_BOLTZSWAPPER_SERVICEMODULE_HPP

#include<memory>

namespace Boltz { class Service; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace BoltzSwapper {

class ServiceModule {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ServiceModule() =delete;

	ServiceModule(ServiceModule&&);
	ServiceModule& operator=(ServiceModule&&);
	~ServiceModule();

	explicit
	ServiceModule( S::Bus& bus
		     , std::unique_ptr<Boltz::Service>
		     );
};

}}}

#endif /* !defined(BOSS_MOD_BOLTZSWAPPER_SERVICEMODULE_HPP) */

