#include"Jsmn/Detail/EndAdvancer.hpp"

namespace Jsmn { namespace Detail {

void EndAdvancer::scan() {
	auto rd = std::make_pair(false, (char) '0');

	if (was_white) {
		was_white = false;
		do {
			rd = sr.read();
			if (!rd.first)
				return;
		} while (std::isspace(rd.second, loc));
		if ( rd.second == '}'
		  || rd.second == ']'
		  || rd.second == '\"'
		   )
			return;
	}

	do {
		rd = sr.read();
		if (!rd.first)
			return;
	} while ( !std::isspace(rd.second, loc)
	       && rd.second != '}'
	       && rd.second != ']'
	       && rd.second != '\"'
		);

	was_white = std::isspace(rd.second, loc);
}

}}
