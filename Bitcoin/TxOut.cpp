#include"Bitcoin/TxOut.hpp"
#include"Bitcoin/le.hpp"
#include"Bitcoin/varint.hpp"

std::ostream& operator<<(std::ostream& os, Bitcoin::TxOut const& v) {
	os << Bitcoin::le(v.amount)
	   << Bitcoin::varint(v.scriptPubKey.size())
	    ;
	for (auto b : v.scriptPubKey)
		os.put(b);
	return os;
}
std::istream& operator>>(std::istream& is, Bitcoin::TxOut& v) {
	auto len = std::size_t();
	is >> Bitcoin::le(v.amount)
	   >> Bitcoin::varint(len)
	    ;
	v.scriptPubKey.resize(len);
	for (auto& b : v.scriptPubKey)
		b = is.get();
	return is;
}
