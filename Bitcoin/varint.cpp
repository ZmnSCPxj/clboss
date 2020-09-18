#include"Bitcoin/varint.hpp"

namespace Bitcoin {

Detail::VarInt varint(std::uint64_t& v) {
	return Detail::VarInt(v);
}
Detail::VarIntConst varint(std::uint64_t const& v) {
	return Detail::VarIntConst(v);
}

}

std::ostream& operator<<(std::ostream& os, Bitcoin::Detail::VarIntConst o) {
	if (o.v < 0xFD)
		os << char(o.v);
	else if (o.v <= 0xFFFF) {
		os << char(0xFD)
		   << char((o.v >> 0) & 0xFF)
		   << char((o.v >> 8) & 0xFF)
		   ;
	} else if (o.v <= 0xFFFFFFFF) {
		os << char(0xFE)
		   << char((o.v >> 0) & 0xFF)
		   << char((o.v >> 8) & 0xFF)
		   << char((o.v >> 16) & 0xFF)
		   << char((o.v >> 24) & 0xFF)
		   ;
	} else {
		os << char(0xFF)
		   << char((o.v >> 0) & 0xFF)
		   << char((o.v >> 8) & 0xFF)
		   << char((o.v >> 16) & 0xFF)
		   << char((o.v >> 24) & 0xFF)
		   << char((o.v >> 32) & 0xFF)
		   << char((o.v >> 40) & 0xFF)
		   << char((o.v >> 48) & 0xFF)
		   << char((o.v >> 56) & 0xFF)
		   ;
	}

	return os;
}

std::istream& operator>>(std::istream& is, Bitcoin::Detail::VarInt o) {
	auto t = char();
	char c[8];

	is >> t;
	if (std::uint8_t(t) < 0xFD)
		o.v = std::uint64_t(std::uint8_t(t));
	else if (std::uint8_t(t) == 0xFD) {
		is >> c[0]
		   >> c[1]
		   ;
		o.v = (std::uint64_t(std::uint8_t(c[0])) << 0)
		    | (std::uint64_t(std::uint8_t(c[1])) << 8)
		    ;
	} else if (std::uint8_t(t) == 0xFE) {
		is >> c[0]
		   >> c[1]
		   >> c[2]
		   >> c[3]
		   ;
		o.v = (std::uint64_t(std::uint8_t(c[0])) << 0)
		    | (std::uint64_t(std::uint8_t(c[1])) << 8)
		    | (std::uint64_t(std::uint8_t(c[2])) << 16)
		    | (std::uint64_t(std::uint8_t(c[3])) << 24)
		    ;
	} else {
		is >> c[0]
		   >> c[1]
		   >> c[2]
		   >> c[3]
		   >> c[4]
		   >> c[5]
		   >> c[6]
		   >> c[7]
		   ;
		o.v = (std::uint64_t(std::uint8_t(c[0])) << 0)
		    | (std::uint64_t(std::uint8_t(c[1])) << 8)
		    | (std::uint64_t(std::uint8_t(c[2])) << 16)
		    | (std::uint64_t(std::uint8_t(c[3])) << 24)
		    | (std::uint64_t(std::uint8_t(c[4])) << 32)
		    | (std::uint64_t(std::uint8_t(c[5])) << 40)
		    | (std::uint64_t(std::uint8_t(c[6])) << 48)
		    | (std::uint64_t(std::uint8_t(c[7])) << 56)
		    ;
	}

	return is;
}
