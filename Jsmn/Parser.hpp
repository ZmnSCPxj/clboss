#ifndef JSMN_PARSER_HPP
#define JSMN_PARSER_HPP

#include<memory>
#include<stdexcept>
#include<string>
#include<utility>
#include<vector>

namespace Jsmn { class Object; }

namespace Jsmn {

/* Thrown on JSON parsing failure.  */
class ParseError : public std::runtime_error {
private:
	std::string input;
	unsigned int i;

	static
	std::string enmessage(std::string const& input, unsigned int i);

public:
	ParseError() =delete;
	ParseError( std::string input_
		  , unsigned int i_
		  ) : std::runtime_error(enmessage(input_, i_))
		    , input(std::move(input_))
		    , i(i_)
		    {
		input = std::move(input_);
		i = i_;
	}
};

/* A stateful jsmn-based parser.
 *
 * If previously-fed data into this parser is not a complete
 * JSON datum, then the feed() member function will return an
 * empty vector.
 * Subsequent feed() calls will append their argument to any
 * incomplete datum until at least one complete datum is
 * formed.
 */
class Parser {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Parser();
	~Parser();

	/* No expected use for these... Disable for now.  */
	Parser(Parser const&) =delete;
	Parser(Parser&&) =delete;

	/* Feeds a string into this parser.*/
	std::vector<Jsmn::Object> feed(std::string const&);
};

}

#endif /* !defined(JSMN_PARSER_HPP) */
