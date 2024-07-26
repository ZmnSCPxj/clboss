#ifndef JSMN_PARSEERROR_HPP
#define JSMN_PARSEERROR_HPP

#include"Util/BacktraceException.hpp"
#include<stdexcept>
#include<string>
#include<utility>

namespace Jsmn {

/* Thrown on JSON parsing failure.  */
class ParseError : public Util::BacktraceException<std::runtime_error> {
private:
	std::string input;
	unsigned int i;

	static
	std::string enmessage(std::string const& input, unsigned int i);

public:
	ParseError() =delete;
	ParseError( std::string input_
		  , unsigned int i_
		  ) : Util::BacktraceException<std::runtime_error>(enmessage(input_, i_))
		    , input(std::move(input_))
		    , i(i_)
		    {
		input = std::move(input_);
		i = i_;
	}
};

}

#endif /* !defined(JSMN_PARSEERROR_HPP) */
