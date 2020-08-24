#ifndef BOSS_MAIN_HPP
#define BOSS_MAIN_HPP

#include<istream>
#include<memory>
#include<ostream>
#include<string>
#include<vector>

namespace Ev { template<typename a> class Io; }

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
	    );
	Main(Main&&);
	~Main();

	Ev::Io<void> run();
};

}

#endif /* !defined(BOSS_MAIN_HPP) */
