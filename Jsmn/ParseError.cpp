#include"Jsmn/ParseError.hpp"

namespace Jsmn {

std::string ParseError::enmessage(std::string const& input, unsigned int i) {
	/* FIXME: Show context around error.  */
	char cs[2];
	cs[0] = input[i];
	cs[1] = 0;
	return std::string("Parse error near character '") + cs
	     + std::string("'")
	     ;
}

}
