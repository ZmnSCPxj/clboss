#ifndef BOSS_MAIN_HPP
#define BOSS_MAIN_HPP

#include<functional>
#include<istream>
#include<memory>
#include<ostream>
#include<string>
#include<vector>

namespace Ev { template<typename a> class Io; }
namespace Net { class Fd; }

namespace Boss {

class Main {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Main() = delete;
	Main( std::vector<std::string> argv
	    , std::istream& cin
	    , std::ostream& cout
	    , std::ostream& cerr
	    , std::function< Net::Fd( std::string const&
				    , std::string const&
				    )
			   > open_rpc_socket
	    );
	Main(Main&&);
	~Main();

	Ev::Io<int> run();
};

}

#endif /* !defined(BOSS_MAIN_HPP) */
