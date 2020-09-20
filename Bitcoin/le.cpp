#include"Bitcoin/le.hpp"

std::ostream& operator<<(std::ostream& os, Bitcoin::Detail::Le16Const o) {
	os << char((o.v >> 0) & 0xFF)
	   << char((o.v >> 8) & 0xFF)
	   ;
	return os;
}
std::istream& operator>>(std::istream& is, Bitcoin::Detail::Le16 o) {
	char c[2];
	is.get(c[0]).get(c[1]);
	o.v = (std::uint16_t(std::uint8_t(c[0])) << 0)
	    | (std::uint16_t(std::uint8_t(c[1])) << 8)
	    ;
	return is;
}
std::ostream& operator<<(std::ostream& os, Bitcoin::Detail::Le32Const o) {
	os << char((o.v >> 0) & 0xFF)
	   << char((o.v >> 8) & 0xFF)
	   << char((o.v >> 16) & 0xFF)
	   << char((o.v >> 24) & 0xFF)
	   ;
	return os;
}
std::istream& operator>>(std::istream& is, Bitcoin::Detail::Le32 o) {
	char c[4];
	is.get(c[0]).get(c[1]).get(c[2]).get(c[3]);
	o.v = (std::uint32_t(std::uint8_t(c[0])) << 0)
	    | (std::uint32_t(std::uint8_t(c[1])) << 8)
	    | (std::uint32_t(std::uint8_t(c[2])) << 16)
	    | (std::uint32_t(std::uint8_t(c[3])) << 24)
	    ;
	return is;
}
std::ostream& operator<<(std::ostream& os, Bitcoin::Detail::Le64Const o) {
	os << char((o.v >> 0) & 0xFF)
	   << char((o.v >> 8) & 0xFF)
	   << char((o.v >> 16) & 0xFF)
	   << char((o.v >> 24) & 0xFF)
	   << char((o.v >> 32) & 0xFF)
	   << char((o.v >> 40) & 0xFF)
	   << char((o.v >> 48) & 0xFF)
	   << char((o.v >> 56) & 0xFF)
	   ;
	return os;
}
std::istream& operator>>(std::istream& is, Bitcoin::Detail::Le64 o) {
	char c[8];
	is.get(c[0]).get(c[1]).get(c[2]).get(c[3])
	  .get(c[4]).get(c[5]).get(c[6]).get(c[7])
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
	return is;
}
std::ostream& operator<<(std::ostream& os, Bitcoin::Detail::LeAmountConst o) {
	auto sats = o.v.to_sat();
	os << Bitcoin::le(sats);
	return os;
}
std::istream& operator>>(std::istream& is, Bitcoin::Detail::LeAmount o) {
	auto sats = std::uint64_t();
	is >> Bitcoin::le(sats);
	o.v = Ln::Amount::sat(sats);
	return is;
}

namespace Bitcoin {

Detail::Le16 le(std::uint16_t& v) {
	return Detail::Le16(v);
}
Detail::Le16Const le(std::uint16_t const& v) {
	return Detail::Le16Const(v);
}
Detail::Le16 le(std::int16_t& v) {
	return Detail::Le16(reinterpret_cast<std::uint16_t&>(v));
}
Detail::Le16Const le(std::int16_t const& v) {
	return Detail::Le16Const(reinterpret_cast<std::uint16_t const&>(v));
}
Detail::Le32 le(std::uint32_t& v) {
	return Detail::Le32(v);
}
Detail::Le32Const le(std::uint32_t const& v) {
	return Detail::Le32Const(v);
}
Detail::Le32 le(std::int32_t& v) {
	return Detail::Le32(reinterpret_cast<std::uint32_t&>(v));
}
Detail::Le32Const le(std::int32_t const& v) {
	return Detail::Le32Const(reinterpret_cast<std::uint32_t const&>(v));
}
Detail::Le64 le(std::uint64_t& v) {
	return Detail::Le64(v);
}
Detail::Le64Const le(std::uint64_t const& v) {
	return Detail::Le64Const(v);
}
Detail::Le64 le(std::int64_t& v) {
	return Detail::Le64(reinterpret_cast<std::uint64_t&>(v));
}
Detail::Le64Const le(std::int64_t const& v) {
	return Detail::Le64Const(reinterpret_cast<std::uint64_t const&>(v));
}
Detail::LeAmount le(Ln::Amount& v) {
	return Detail::LeAmount(v);
}
Detail::LeAmountConst le(Ln::Amount const& v) {
	return Detail::LeAmountConst(v);
}

}
