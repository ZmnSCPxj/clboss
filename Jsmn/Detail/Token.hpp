#ifndef JSMN_DETAIL_TOKEN_HPP
#define JSMN_DETAIL_TOKEN_HPP

#include"Jsmn/Detail/Type.hpp"

namespace Jsmn { namespace Detail {

struct Token {
	Type type;
	int start;
	int end;
	int size;

	static void next(Token const*& tokptr);
};

}}

#endif /* !defined(JSMN_DETAIL_TOKEN_HPP) */
