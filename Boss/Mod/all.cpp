#include"Boss/Mod/AutoDisconnector.hpp"
#include"Boss/Mod/BlockTracker.hpp"
#include"Boss/Mod/ChannelCandidatePreinvestigator.hpp"
#include"Boss/Mod/ChannelCandidateInvestigator/Main.hpp"
#include"Boss/Mod/ChannelCreationDecider.hpp"
#include"Boss/Mod/ChannelCreator/Main.hpp"
#include"Boss/Mod/ChannelFinderByPopularity.hpp"
#include"Boss/Mod/ChannelFundsComputer.hpp"
#include"Boss/Mod/ConnectFinderByDns.hpp"
#include"Boss/Mod/ConnectFinderByHardcode.hpp"
#include"Boss/Mod/Connector.hpp"
#include"Boss/Mod/CommandReceiver.hpp"
#include"Boss/Mod/InitialConnect.hpp"
#include"Boss/Mod/Initiator.hpp"
#include"Boss/Mod/InternetConnectionMonitor.hpp"
#include"Boss/Mod/JsonOutputter.hpp"
#include"Boss/Mod/ListfundsAnnouncer.hpp"
#include"Boss/Mod/ListpeersAnalyzer.hpp"
#include"Boss/Mod/ListpeersAnnouncer.hpp"
#include"Boss/Mod/Manifester.hpp"
#include"Boss/Mod/NeedsConnectSolicitor.hpp"
#include"Boss/Mod/OnchainFeeMonitor.hpp"
#include"Boss/Mod/OnchainFundsAnnouncer.hpp"
#include"Boss/Mod/Reconnector.hpp"
#include"Boss/Mod/StatusCommand.hpp"
#include"Boss/Mod/Timers.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Mod/all.hpp"
#include<vector>

namespace {

class All {
private:
	std::vector<std::shared_ptr<void>> modules;

public:
	template<typename M, typename... As>
	std::shared_ptr<M> install(As&&... as) {
		auto ptr = std::make_shared<M>(as...);
		modules.push_back(std::shared_ptr<void>(ptr));
		return ptr;
	}
};

}

namespace Boss { namespace Mod {

std::shared_ptr<void> all( std::ostream& cout
			 , S::Bus& bus
			 , Ev::ThreadPool& threadpool
			 , std::function< Net::Fd( std::string const&
						 , std::string const&
						 )
					> open_rpc_socket
			 ) {
	auto all = std::make_shared<All>();

	/* Basic.  */
	auto waiter = all->install<Waiter>(bus);
	auto imon = all->install<InternetConnectionMonitor>( bus
							   , threadpool
							   , *waiter
							   );
	all->install<JsonOutputter>(cout, bus);
	all->install<CommandReceiver>(bus);

	/* Startup.  */
	all->install<Manifester>(bus);
	all->install<Initiator>(bus, threadpool, std::move(open_rpc_socket));

	/* Regular timers.  */
	all->install<BlockTracker>(bus);
	all->install<Timers>(bus, *waiter);

	/* Connection wrangling.  */
	all->install<InitialConnect>(bus);
	all->install<Connector>(bus);
	all->install<NeedsConnectSolicitor>(bus);
	all->install<ConnectFinderByDns>(bus);
	all->install<ConnectFinderByHardcode>(bus);
	all->install<Reconnector>(bus);
	all->install<AutoDisconnector>(bus);

	/* Status monitors.  */
	all->install<ChannelFundsComputer>(bus);
	all->install<ListpeersAnnouncer>(bus);
	all->install<ListpeersAnalyzer>(bus);
	all->install<OnchainFeeMonitor>(bus, *waiter);
	all->install<ListfundsAnnouncer>(bus);
	all->install<OnchainFundsAnnouncer>(bus);

	/* Channel creation wrangling.  */
	all->install<ChannelFinderByPopularity>(bus, *waiter);
	all->install<ChannelCandidatePreinvestigator>(bus);
	auto investigator = all->install< ChannelCandidateInvestigator::Main
					>(bus, *imon);
	all->install<ChannelCreationDecider>(bus);
	all->install<ChannelCreator::Main>(bus, *waiter, *investigator);

	/* Status.  */
	all->install<StatusCommand>(bus);

	return all;
}

}}
