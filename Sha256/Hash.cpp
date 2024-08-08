#include"Sha256/Hash.hpp"
#include"Util/Str.hpp"
#include<basicsecure.h>
#include<stdexcept>

namespace {

std::uint8_t const zero[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

}

namespace Sha256 {

bool Hash::valid_string(std::string const& s) {
	return s.size() == 64 && Util::Str::ishex(s);
}
Hash::Hash(std::string const& s) {
	auto bytes = Util::Str::hexread(s);
	if (bytes.size() != 32)
		throw Util::BacktraceException<std::invalid_argument>("Hashes must be 32 bytes.");
	pimpl = std::make_shared<Impl>();
	for (auto i = std::size_t(0); i < 32; ++i)
		pimpl->d[i] = bytes[i];
}

Hash::operator std::string() const {
	if (!pimpl)
		return "0000000000000000000000000000000000000000000000000000000000000000";

	return Util::Str::hexdump(pimpl->d, 32);
}
Hash::operator bool() const {
	if (!pimpl)
		return false;

	return !basicsecure_eq(zero, pimpl->d, 32);
}
bool Hash::operator==(Hash const& i) const {
	auto a = pimpl ? pimpl->d : zero;
	auto b = i.pimpl ? i.pimpl->d : zero;
	return basicsecure_eq(a, b, 32);
}

}
