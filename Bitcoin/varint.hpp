#ifndef BITCOIN_VARINT_HPP
#define BITCOIN_VARINT_HPP

#include<cstdint>
#include<iostream>

namespace Bitcoin { namespace Detail { class VarInt; }}
namespace Bitcoin { namespace Detail { class VarIntConst; }}

namespace Bitcoin {

/** Bitcoin::varint
 *
 * @brief wraps a uint64_t so that when it is
 * serialized with normal C++ streams, it is
 * encoded with Bitcoin `CompactSize`.
 *
 * @desc intended use is:
 *
 *     std::cout << Bitcoin::varint(number);
 */
Detail::VarInt varint(std::uint64_t& v);
Detail::VarIntConst varint(std::uint64_t const& v);

}

std::ostream& operator<<(std::ostream&, Bitcoin::Detail::VarIntConst);
std::istream& operator>>(std::istream&, Bitcoin::Detail::VarInt);

namespace Bitcoin { namespace Detail {

class VarInt {
private:
	std::uint64_t& v;

	friend
	std::istream& ::operator>>(std::istream&, Bitcoin::Detail::VarInt);

	friend class VarIntConst;

public:
	VarInt(std::uint64_t& v_) : v(v_) { }
};

class VarIntConst {
private:
	std::uint64_t v;

	friend
	std::ostream& ::operator<<(std::ostream&, Bitcoin::Detail::VarIntConst);

public:
	VarIntConst(std::uint64_t const& v_) : v(v_) { }
	VarIntConst(VarInt const& o) : v(o.v) { }
};

}}

#endif /* !defuned(BITCOIN_VARINT_HPP) */
