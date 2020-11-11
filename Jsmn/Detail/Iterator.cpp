#include"Jsmn/Detail/Iterator.hpp"
#include"Jsmn/Detail/ParseResult.hpp"
#include"Jsmn/Detail/Token.hpp"
#include"Jsmn/Object.hpp"

namespace Jsmn { namespace Detail {

Iterator::Iterator() : r(), i(0) { }

Iterator& Iterator::operator++() {
	Token const* tok = &r->tokens[i];
	Token const* ntok = tok;
	Token::next(ntok);
	i += (ntok - tok);

	return *this;
}
Jsmn::Object Iterator::operator*() const {
	return Jsmn::Object(r, i);
}

}}
