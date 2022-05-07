#ifndef JSMN_PARSER_HPP
#define JSMN_PARSER_HPP

#include<memory>
#include<string>
#include<utility>
#include<vector>
#include"Jsmn/ParseError.hpp"

namespace Jsmn { class Object; }

namespace Jsmn {

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
