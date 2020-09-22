#include"Bitcoin/TxId.hpp"

std::ostream& operator<<(std::ostream& os, Bitcoin::TxId const& id) {
	std::uint8_t hash[32];
	id.hash.to_buffer(hash);
	for (auto i = std::size_t(0); i < sizeof(hash); ++i)
		/* TxIds are serialized in "correct" order.  */
		os << ((char) hash[sizeof(hash) - i - 1]);
	return os;
}
std::istream& operator>>(std::istream& is, Bitcoin::TxId& id) {
	std::uint8_t hash[32];
	for (auto i = std::size_t(0); i < sizeof(hash); ++i)
		/* TxIds are serialized in "correct" order.  */
		hash[sizeof(hash) - i - 1] = std::uint8_t(is.get());
	id.hash.from_buffer(hash);
	return is;
}

namespace Bitcoin {

TxId::TxId(std::string const& s) : hash(s) { }
TxId::operator std::string() const {
	return std::string(hash);
}
TxId::TxId(Sha256::Hash hash_) {
	std::uint8_t buf[32];
	hash_.to_buffer(buf);
	for (auto i = std::size_t(0); i < sizeof(buf) / 2; ++i)
		std::swap(buf[i], buf[sizeof(buf) - 1 - i]);
	hash.from_buffer(buf);
}

}
