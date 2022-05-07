#include"Jsmn/Detail/DatumIdentifier.hpp"
#include"Jsmn/Detail/ParseResult.hpp"
#include"Jsmn/Detail/Token.hpp"
#include"Jsmn/Detail/Type.hpp"
#include"Jsmn/Object.hpp"
#include"Jsmn/ParserExposedBuffer.hpp"
#include"Util/make_unique.hpp"
#include<algorithm>
#include<assert.h>

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

}

namespace Jsmn {

class ParserExposedBuffer::Impl {
private:
	std::vector<char> buffer;

	/* The start of parsing.  */
	std::size_t start_idx;
	/* The end of parsing, as identified by the datum-identifier.  */
	std::size_t end_idx;
	/* The end of already-loaded data.  */
	std::size_t load_idx;

	jsmn_parser base;
	std::vector<jsmntok_t> toks;
	Jsmn::Detail::DatumIdentifier datum_ender;

public:
	Impl() {
		start_idx = 0;
		end_idx = 0;
		load_idx = 0;

		jsmn_init(&base);
		toks.resize(4);
	}
	void load_buffer( std::size_t reserve
			, std::function<std::size_t(char*)> f
			) {
		if (load_idx + reserve > buffer.size()) {
			buffer.resize(load_idx + reserve);
		}
		load_idx += f(&buffer[load_idx]);
	}
	bool can_parse_buffer() const {
		return end_idx != load_idx;
	}

private:
	/*~ Parses a single JSON datum.
	 *
	 * The loaded buffer might contain an incomplete
	 * JSON object, in which case it will be left in
	 * the start_idx -> end_idx region, with the
	 * end_idx being equal to load_idx.
	 *
	 * If this returns nullptr, then end_idx == load_idx.
	 *
	 * Return type should really be std::optional<Object>, but we
	 * are constrained to use C++11 for (hopefully) better portability
	 * into weirder systems, so ---
	 */
	std::unique_ptr<Object> try_parse() {
		/* First, check if there is a complete
		 * datum.
		 */
		while (end_idx < load_idx) {
			if (datum_ender.feed(buffer[end_idx++]))
				break;
			if (end_idx == load_idx)
				return nullptr;
		}
		for (;;) {
			auto res = jsmn_parse( &base
					     , &buffer[start_idx], end_idx - start_idx
					     , &toks[0], toks.size()
					     );
			if (res > 0) {
				assert(base.pos + start_idx <= end_idx);
				auto pr = Detail::ParseResult();
				auto text = std::string( &buffer[start_idx]
						       , &buffer[start_idx + base.pos]
						       );
				pr.orig_string = std::move(text);
				pr.tokens.resize(res);
				for (auto i = 0; i < res; ++i)
					pr.tokens[i] = token_convert(toks[i]);

				start_idx += base.pos;
				jsmn_init(&base);

				auto ppr = std::make_shared<Detail::ParseResult>(std::move(pr));
				auto o = Object(std::move(ppr), 0);
				return Util::make_unique<Object>(std::move(o));
			}
			if (res == 0) {
				/* Only whitespace.  Eat them all.  */
				assert(base.pos + start_idx <= end_idx);
				start_idx += base.pos;
				jsmn_init(&base);
				assert(end_idx == load_idx);
				return nullptr;
			}

			switch (res) {
			case JSMN_ERROR_NOMEM:
				toks.resize(toks.size() * 2 + 1);
				break;
			case JSMN_ERROR_INVAL: {
				auto text = std::string( &buffer[start_idx]
						       , &buffer[end_idx]
						       );
				throw ParseError(text, base.pos);
			} break;
			case JSMN_ERROR_PART:
				assert(end_idx == load_idx);
				return nullptr;
			}
		}
	}

	/*~ Move the loaded part of the buffer back to the
	 * start of the buffer, if it makes sense.
	 *
	 * We want to avoid the degenerate case where a
	 * tiny complete JSON datum is followed by a large
	 * incomplete JSON datum.
	 * In that case, retain our start_idx instead of
	 * moving the data.
	 */
	void move_loaded() {
		if ((load_idx - start_idx) > start_idx)
			return;
		std::copy( &buffer[start_idx], &buffer[load_idx]
			 , &buffer[0]
			 );
		load_idx -= start_idx;
		end_idx -= start_idx;
		start_idx = 0;
	}
public:
	std::vector<Object> parse_buffer() {
		auto ret = std::vector<Object>();

		if (!can_parse_buffer())
			return ret;

		while (auto tmp = try_parse())
			ret.emplace_back(std::move(*tmp));
		assert(!can_parse_buffer());

		move_loaded();
		return ret;
	}
};

ParserExposedBuffer::ParserExposedBuffer()
	: pimpl(Util::make_unique<Impl>()) {}

ParserExposedBuffer::~ParserExposedBuffer() =default;

void ParserExposedBuffer::load_buffer( std::size_t reserve
				     , std::function<std::size_t(char*)> f
				     ) {
	return pimpl->load_buffer(reserve, std::move(f));
}
bool ParserExposedBuffer::can_parse_buffer() const {
	return pimpl->can_parse_buffer();
}
std::vector<Jsmn::Object> ParserExposedBuffer::parse_buffer() {
	return pimpl->parse_buffer();
}

}
