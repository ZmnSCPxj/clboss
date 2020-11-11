#ifndef JSMN_DETAIL_ITERATOR_HPP
#define JSMN_DETAIL_ITERATOR_HPP

#include<memory>

namespace Jsmn { namespace Detail { struct ParseResult; }}
namespace Jsmn { namespace Detail { struct Token; }}
namespace Jsmn { class Object; }

namespace Jsmn { namespace Detail {

class Iterator {
private:
	std::shared_ptr<Detail::ParseResult> r;
	unsigned int i;

	friend class Jsmn::Object;

	Iterator( std::shared_ptr<Detail::ParseResult> r_
		, unsigned int i_
		) : r(r_), i(i_) { }

public:
	Iterator();
	Iterator(Iterator const&) =default;

	bool operator==(Iterator const& o) const {
		return r == o.r && i == o.i;
	}
	bool operator!=(Iterator const& o) const {
		return !(*this == o);
	}
	Iterator& operator++();
	Iterator operator++(int) {
		auto it = *this;
		++(*this);
		return it;
	}

	Jsmn::Object operator*() const;
};

}}

#endif /* !defined(JSMN_DETAIL_ITERATOR_HPP) */
