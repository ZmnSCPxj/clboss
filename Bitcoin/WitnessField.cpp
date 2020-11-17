#include"Bitcoin/WitnessField.hpp"
#include"Bitcoin/varint.hpp"

std::ostream& operator<<(std::ostream& os, Bitcoin::WitnessField const& v) {
	os << Bitcoin::varint(v.witnesses.size());
	for (auto const& w : v.witnesses) {
		os << Bitcoin::varint(w.size());
		for (auto b : w)
			os.put(b);
	}
	return os;
}
std::istream& operator>>(std::istream& is, Bitcoin::WitnessField& v) {
	auto len = std::uint64_t();
	is >> Bitcoin::varint(len);
	v.witnesses.resize(std::size_t(len));
	for (auto& w : v.witnesses) {
		is >> Bitcoin::varint(len);
		w.resize(len);
		for (auto& b : w)
			b = is.get();
	}
	return is;
}
