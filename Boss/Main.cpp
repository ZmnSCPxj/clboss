#include<Boss/Main.hpp>
#include<Ev/Io.hpp>
#include<Util/make_unique.hpp>
#include<assert.h>
#include<iostream>
#include<string>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

namespace Boss {

class Main::Impl {
private:
	std::istream& cin;
	std::ostream& cout;
	std::ostream& cerr;

	std::string argv0;
	bool is_version;
	bool is_help;

public:
	Impl( std::vector<std::string> argv
	    , std::istream& cin_
	    , std::ostream& cout_
	    , std::ostream& cerr_
	    ) : cin(cin_)
	      , cout(cout_)
	      , cerr(cerr_)
	      , is_version(false)
	      , is_help(false)
	      {
		assert(argv.size() >= 1);
		argv0 = argv[0];
		if (argv.size() >= 2) {
			auto argv1 = argv[1];
			if (argv1 == "--version" || argv1 == "-V")
				is_version = true;
			else
				is_help = true;
		}
	}

	Ev::Io<void> run() {
		if (is_version) {
			cout << PACKAGE_STRING << std::endl;
			return Ev::lift();
		} else if (is_help) {
			cout << "Usage: add --plugin=" << argv0 << " to your lightningd command line or configuration file" << std::endl
			     << std::endl
			     << "Options:" << std::endl
			     << " --version, -V      Show version." << std::endl
			     << " --help, -H         Show this help." << std::endl
			     << std::endl
			     << "CLBOSS is proudly Free and Open Source Software" << std::endl
			     << "It is provided WITHOUT ANY WARRANTIES" << std::endl
			     << std::endl
			     << "Send bug reports to: " << PACKAGE_BUGREPORT << std::endl
			     ;
			return Ev::lift();
		}

		/* TODO */
		cout << "Hello World" << std::endl;
		return Ev::lift();
	}
};

Main::Main( std::vector<std::string> argv
	  , std::istream& cin
	  , std::ostream& cout
	  , std::ostream& cerr
	  ) : pimpl(Util::make_unique<Impl>(std::move(argv), cin, cout, cerr))
	    { }
Main::Main(Main&& o) : pimpl(std::move(o.pimpl)) { }
Main::~Main() { }

Ev::Io<void> Main::run() {
	assert(pimpl);
	return pimpl->run();
}

}
