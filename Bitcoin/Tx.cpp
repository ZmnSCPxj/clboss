#include"Bitcoin/Tx.hpp"
#include"Bitcoin/TxId.hpp"
#include"Bitcoin/le.hpp"
#include"Bitcoin/varint.hpp"
#include"Sha256/HasherStream.hpp"
#include"Sha256/fun.hpp"
#include"Util/Str.hpp"
#include<algorithm>
#include<sstream>
#include<stdexcept>

std::ostream& operator<<(std::ostream& os, Bitcoin::Tx const& v) {
	os << Bitcoin::le(v.nVersion);

	/* Should we serialize as segwit?  */
	auto segwit = std::any_of
		( v.inputs.begin(), v.inputs.end()
		, [](Bitcoin::TxIn const& i) { return !i.witness.empty(); }
		);

	if (segwit) {
		/* Flag and marker.  */
		os.put(0x00);
		os.put(0x01);
	}

	os << Bitcoin::varint(v.inputs.size());
	for (auto const& i : v.inputs)
		os << i;

	os << Bitcoin::varint(v.outputs.size());
	for (auto const& o : v.outputs)
		os << o;

	if (segwit) {
		for (auto const& i : v.inputs)
			os << i.witness;
	}

	os << Bitcoin::le(v.nLockTime);

	return os;
}

std::istream& operator>>(std::istream& is, Bitcoin::Tx& v) {
	auto len = std::uint64_t();

	is >> Bitcoin::le(v.nVersion)
	   >> Bitcoin::varint(len)
	    ;
	auto segwit = (len == 0);

	if (segwit) {
		/* Marker and *actual* length.  */
		if (is.get() != 0x01) {
			is.setstate(std::ios_base::failbit);
			return is;
		}
		is >> Bitcoin::varint(len);
	}
	v.inputs.resize(std::size_t(len));
	for (auto& i : v.inputs)
		is >> i;

	is >> Bitcoin::varint(len);
	v.outputs.resize(std::size_t(len));
	for (auto& o : v.outputs)
		is >> o;

	if (segwit) {
		for (auto& i : v.inputs)
			is >> i.witness;
	} else {
		for (auto& i : v.inputs)
			i.witness.witnesses.clear();
	}

	is >> Bitcoin::le(v.nLockTime);

	return is;
}

namespace Bitcoin {

Tx::Tx(std::string const& s) {
	auto buf = Util::Str::hexread(s);
	auto str = std::string(buf.begin(), buf.end());
	auto is = std::istringstream(std::move(str));
	is >> *this;
	if (!is.good())
		throw Util::BacktraceException<std::invalid_argument>("Bitcoin::Tx: invalid hex string input.");
	if (is.get() != std::char_traits<char>::eof())
		throw Util::BacktraceException<std::invalid_argument>("Bitcoin::Tx: input string too long.");
}

Bitcoin::TxId Tx::get_txid() const {
	/* Copy this, then strip witnesses.
	 * Inefficient, but easy.
	 */
	auto tmp = *this;
	for (auto& i : tmp.inputs)
		i.witness.witnesses.clear();

	/* Create a sha256 hasher stream.  */
	Sha256::HasherStream hasher;
	hasher << tmp;

	/* Double-hash.  */
	return Bitcoin::TxId(
		Sha256::fun(std::move(hasher).finalize())
	);
}

Tx::operator std::string() const {
	auto os = std::ostringstream();
	os << *this;
	auto str = os.str();
	return Util::Str::hexdump(&str[0], str.size());
}

}
