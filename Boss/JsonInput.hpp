#ifndef BOSS_JSONINPUT_HPP
#define BOSS_JSONINPUT_HPP

#include<istream>
#include<memory>

namespace Ev { template<typename a> class Io; }
namespace Ev { class ThreadPool; }
namespace S { class Bus; }

namespace Boss {

/** class JsonInput
 *
 * @brief special module that takes the cin and
 * emits Boss::Msg::JsonCin messages for each
 * JSON object it receivs.
 *
 * @desc Unlike other modules, this module has a
 * run function.
 * When the run function action returns, the input
 * has reached end-of-file and the rest of the BOSS
 * needs to shut down.
 */
class JsonInput {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	explicit
	JsonInput( Ev::ThreadPool& threadpool
		 , std::istream& cin
		 , S::Bus& bus
		 );
	JsonInput(JsonInput&&);
	~JsonInput();

	Ev::Io<void> run();
};

}

#endif /* ~defined(BOSS_JSONINPUT_HPP) */
