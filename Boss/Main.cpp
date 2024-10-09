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
#include<sstream>
#include<signal.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

std::string g_argv0;

namespace Boss {

class Main::Impl {
private:
	std::istream& cin;
	std::ostream& cout;
	std::ostream& cerr;
	std::function< Net::Fd( std::string const&
			      , std::string const&
			      )
		     > open_rpc_socket;

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
	    , std::function< Net::Fd( std::string const&
				    , std::string const&
				    )
			   > open_rpc_socket_
	    ) : cin(cin_)
	      , cout(cout_)
	      , cerr(cerr_)
	      , open_rpc_socket(std::move(open_rpc_socket_))
	      , exit_code(0)
	      , is_version(false)
	      , is_help(false)
	      {
		assert(argv.size() >= 1);
		argv0 = argv[0];
		g_argv0 = argv[0];
		if (argv.size() >= 2) {
			auto argv1 = argv[1];
			if (argv1 == "--version" || argv1 == "-V")
				is_version = true;
			else if (argv1 == "--debugger") {
				auto os = std::ostringstream();
				os << "${DEBUG_TERM:-gnome-terminal --} gdb -ex 'attach "
				   << getpid() << "' " << argv0 << " >/dev/null &"
				   ;
				if (system(os.str().c_str()))
					;
				kill(getpid(), SIGSTOP);
			} else if (argv1 == "--developer") {
				/* In the future we might want to do something, but for now
				 * accept and don't fail with usage message
				 */
			} else {
				std::cerr << argv0 << ":"
					  << " Unrecognized option: " << argv1
					  << std::endl;
				is_help = true;
			}
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
			     << " --debugger         Run gdb in a new term attached to clboss." << std::endl
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
		modules = Boss::Mod::all( cout
					, *bus
					, *threadpool
					, open_rpc_socket
					);

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
	  , std::function< Net::Fd( std::string const&
				  , std::string const&
				  )
			 > open_rpc_socket
	  ) : pimpl(Util::make_unique<Impl>( std::move(argv)
					   , cin
					   , cout
					   , cerr
					   , std::move(open_rpc_socket)
					   ))
	    { }
Main::Main(Main&& o) : pimpl(std::move(o.pimpl)) { }
Main::~Main() { }

Ev::Io<int> Main::run() {
	assert(pimpl);
	return pimpl->run();
}

}
