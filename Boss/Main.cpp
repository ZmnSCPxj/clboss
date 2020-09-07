#include<assert.h>
#include<iostream>
#include<string>
#include"Boss/JsonInput.hpp"
#include"Boss/Main.hpp"
#include"Boss/Mod/all.hpp"
#include"Boss/Msg/Begin.hpp"
#include"Boss/Shutdown.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

namespace Boss {

class Main::Impl {
private:
	std::istream& cin;
	std::ostream& cout;
	std::ostream& cerr;

	std::unique_ptr<S::Bus> bus;
	std::unique_ptr<Ev::ThreadPool> threadpool;
	std::unique_ptr<Boss::JsonInput> jsoninput;
	std::shared_ptr<void> modules;

	int exit_code;

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
	      , exit_code(0)
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

	Ev::Io<int> run() {
		if (is_version) {
			cout << PACKAGE_STRING << std::endl;
			return Ev::lift(0);
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
			return Ev::lift(0);
		}

		/* Build our components.  */
		bus = Util::make_unique<S::Bus>();
		threadpool = Util::make_unique<Ev::ThreadPool>();
		jsoninput = Util::make_unique<Boss::JsonInput>(
			*threadpool, cin, *bus
		);
		modules = Boss::Mod::all(cout, *bus, *threadpool);

		return Ev::yield().then([this]() {
			/* Begin.  */
			return bus->raise(Boss::Msg::Begin());
		}).then([this]() {
			/* Main loop.  */
			return jsoninput->run().catching<std::exception>([this](std::exception const& e) {
				cerr << "Uncaught exception: " << e.what() << std::endl;
				exit_code = 1;
				return Ev::lift();
			});
		}).then([this]() {
			/* Finish.  */
			return bus->raise(Boss::Shutdown());
		}).then([this]() {
			return Ev::lift(exit_code);
		});
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

Ev::Io<int> Main::run() {
	assert(pimpl);
	return pimpl->run();
}

}
