#ifndef BOSS_MOD_JSONOUTPUTTER_HPP
#define BOSS_MOD_JSONOUTPUTTER_HPP

#include<ostream>
#include<string>
#include<queue>

namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::JsonOutputter
 *
 * @brief module that outputs Boss::Msg::JsonCout
 * objects.
 */
class JsonOutputter {
private:
	std::ostream& cout;
	std::queue<std::string> outs;

	Ev::Io<void> loop();
public:
	JsonOutputter( std::ostream& cout_
		     , S::Bus& bus_
		     );
};

}}

#endif /* !defined(BOSS_MOD_JSONOUTPUTTER_HPP) */
