#include<assert.h>
#include"Jsmn/Detail/EndAdvancer.hpp"
#include"Jsmn/Detail/ParseResult.hpp"
#include"Jsmn/Detail/Token.hpp"
#include"Jsmn/Detail/Type.hpp"
#include"Jsmn/Object.hpp"
#include"Jsmn/Parser.hpp"
#include"Util/make_unique.hpp"

/* jsmn has all its code in the jsmn.h header, so instantiate all its
 * code into this compilation unit.
 */
#define JSMN_STATIC 1		/* Everything in this compilation unit.  */
#undef JSMN_HEADER		/* Not header-only.  */
#define JSMN_PARENT_LINKS 1	/* Faster parsing for more memory use.  */
#define JSMN_STRICT 1		/* Reject sloppy JSON.  */
# include <jsmn.h>

namespace {

/* Convert from jsmn.h types to Jsmn::Detail::Type.  */
Jsmn::Detail::Type type_convert(jsmntype_t t) {
	switch (t) {
	case JSMN_UNDEFINED: return Jsmn::Detail::Undefined;
	case JSMN_OBJECT: return Jsmn::Detail::Object;
	case JSMN_ARRAY: return Jsmn::Detail::Array;
	case JSMN_STRING: return Jsmn::Detail::String;
	case JSMN_PRIMITIVE: return Jsmn::Detail::Primitive;
	}
	abort();
}
/* Convert from jsmn.h tokens to Jsmn::Detail::Token.  */
Jsmn::Detail::Token token_convert(jsmntok_t const& tok) {
	auto ret = Jsmn::Detail::Token();
	ret.type = type_convert(tok.type);
	ret.start = tok.start;
	ret.end = tok.end;
	ret.size = tok.size;
	return ret;
}

/* A Jsmn::Detail::SourceReader which reads the JSON string
 * currently stored in the Jsmn::Parser implementation.
 */
class ParserSourceReader : public Jsmn::Detail::SourceReader {
private:
	std::string const& input;
	unsigned int i;
public:
	ParserSourceReader(std::string const& input_) : input(input_), i(0) { }
	std::pair<bool, char> read() {
		if (i >= input.size())
			return std::make_pair(false, '0');
		return std::make_pair(true, input[i++]);
	}
	/* Indicates that a number of characters have been removed
	 * from the front of the input buffer.
	 */
	void reset(unsigned int to_consume) {
		i -= to_consume;
	}
	/* Determine how much of the input to feed.  */
	unsigned int input_size() const {
		return i;
	}
};

}

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

class Parser::Impl {
private:
	std::string input;
	ParserSourceReader psr;
	std::vector<jsmntok_t> toks;
	jsmn_parser base;

	void reset(unsigned int to_consume) {
		input.erase(0, to_consume);
		psr.reset(to_consume);
		jsmn_init(&base);
	}

public:
	Impl() : psr(input) {
		toks.resize(4);
		reset(0);
	}

	void append(std::string const& s) {
		input += s;
	}

	/* Parses some data from the input, consuming them if one
	 * complete datum is found.
	 *
	 * Return type should really be std::optional<Object>, but we
	 * are constrained to use C++11 for (hopefully) better portability
	 * into weirder systems, so ---
	 */
	std::unique_ptr<Object> try_parse() {
		auto ender = Detail::EndAdvancer(psr);
		for (;;) {
			/* Advance things.  */
			ender.scan();

			auto res = jsmn_parse( &base
					     , input.data(), psr.input_size()
					     , &toks[0], toks.size()
					     );
			if (res > 0) {
				auto pr = Detail::ParseResult();
				auto to_consume = base.pos;
				pr.orig_string = std::string(&input[0], to_consume);
				pr.tokens.resize(res);
				for (auto i = 0; i < res; ++i) {
					pr.tokens[i] = token_convert(toks[i]);
				}

				/* Reset parsing state.  */
				reset(to_consume);

				/* Construct object wrapper.  */
				auto ppr = std::make_shared<Detail::ParseResult>(std::move(pr));
				auto o = Object(std::move(ppr) , 0);
				return Util::make_unique<Object>(std::move(o));
			}

			if (res == 0) {
				/* Whitespace only. Eat them all.  */
				reset(base.pos);
				return nullptr;
			}

			switch (res) {
			case JSMN_ERROR_NOMEM:
				toks.resize(toks.size() * 2 + 1);
				break;
			case JSMN_ERROR_INVAL:
				throw ParseError(input, base.pos);
			case JSMN_ERROR_PART:
				if (psr.input_size() == input.size())
					return nullptr;
				/* Let the end advancer continue.  */
				continue;
			}
		}
	}
};

Parser::Parser()
	: pimpl(Util::make_unique<Impl>()) { }
Parser::~Parser() { }

std::vector<Jsmn::Object> Parser::feed(std::string const& s) {
	pimpl->append(s);

	auto ret = std::vector<Jsmn::Object>();
	auto tmp = std::unique_ptr<Jsmn::Object>();
	do {
		tmp = pimpl->try_parse();
		if (tmp)
			ret.emplace_back(std::move(*tmp));
	} while (tmp);
	return ret;
}

}
