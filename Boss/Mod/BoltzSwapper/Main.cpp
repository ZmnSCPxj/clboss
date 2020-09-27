#include"Boss/Mod/BoltzSwapper/Env.hpp"
#include"Boss/Mod/BoltzSwapper/Main.hpp"
#include"Boss/Mod/BoltzSwapper/ServiceCreator.hpp"
#include"Util/make_unique.hpp"

namespace {

auto const boltz_instances = std::map<Boss::Msg::Network, std::vector<std::string>>
{ { Boss::Msg::Network_Bitcoin
  , { "https://boltz.exchange/api"
    }
  }
, { Boss::Msg::Network_Testnet
  , { "https://testnet.boltz.exchange/api"
    }
  }
};

}

namespace Boss { namespace Mod { namespace BoltzSwapper {

class Main::Impl {
private:
	Env env;
	ServiceCreator creator;

public:
	Impl(S::Bus& bus, Ev::ThreadPool& threadpool)
		: env(bus)
		, creator(bus, threadpool, env, boltz_instances)
		{ }
};

Main::Main(Main&&) =default;
Main& Main::operator=(Main&&) =default;
Main::~Main() =default;

Main::Main( S::Bus& bus
	  , Ev::ThreadPool& threadpool
	  ) : pimpl(Util::make_unique<Impl>(bus, threadpool))
	    { }

}}}

