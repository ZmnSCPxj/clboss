#include"Jsmn/Detail/Token.hpp"

namespace Jsmn { namespace Detail {

void Token::next(Token const*& tokptr) {
	auto t = (Token const*) nullptr;
	auto i = (int) 0;
	for (t = tokptr + 1, i = 0; i < tokptr->size; ++i)
		next(t);
	tokptr = t;
}

}}
