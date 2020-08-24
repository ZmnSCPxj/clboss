#ifndef JSMN_DETAIL_PARSERESULT_HPP
#define JSMN_DETAIL_PARSERESULT_HPP

#include<string>
#include<vector>
#include"Jsmn/Detail/Token.hpp"

namespace Jsmn { namespace Detail {

struct ParseResult {
	std::string orig_string;
	std::vector<Token> tokens;
};

}}

#endif /* !defined(JSMN_DETAIL_PARSERESULT_HPP) */
