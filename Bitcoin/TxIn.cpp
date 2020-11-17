#include"Bitcoin/TxIn.hpp"
#include"Bitcoin/le.hpp"
#include"Bitcoin/varint.hpp"

std::ostream& operator<<(std::ostream& os, Bitcoin::TxIn const& v) {
	os << v.prevTxid
	   << Bitcoin::le(v.prevOut)
	   << Bitcoin::varint(v.scriptSig.size())
	    ;
	for (auto b : v.scriptSig)
		os.put(b);
	os << Bitcoin::le(v.nSequence);
	return os;
}
std::istream& operator>>(std::istream& is, Bitcoin::TxIn& v) {
	auto scriptSigLen = std::uint64_t();
	is >> v.prevTxid
	   >> Bitcoin::le(v.prevOut)
	   >> Bitcoin::varint(scriptSigLen)
	    ;
	v.scriptSig.resize(std::size_t(scriptSigLen));
	for (auto& b : v.scriptSig)
		b = is.get();
	is >> Bitcoin::le(v.nSequence);
	return is;
}
