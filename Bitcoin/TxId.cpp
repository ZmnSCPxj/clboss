#include"Bitcoin/TxId.hpp"

std::ostream& operator<<(std::ostream& os, Bitcoin::TxId const& id) {
	std::uint8_t hash[32];
	id.hash.to_buffer(hash);
	for (auto i = std::size_t(0); i < sizeof(hash); ++i)
		/* TxIds are serialized in reverse order.  */
		os << ((char) hash[sizeof(hash) - i - 1]);
	return os;
}
std::istream& operator>>(std::istream& is, Bitcoin::TxId& id) {
	std::uint8_t hash[32];
	for (auto i = std::size_t(0); i < sizeof(hash); ++i)
		/* TxIds are serialized in reverse order.  */
		hash[sizeof(hash) - i - 1] = std::uint8_t(is.get());
	id.hash.from_buffer(hash);
	return is;
}
