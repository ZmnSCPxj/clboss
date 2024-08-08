#include"Ripemd160/Hash.hpp"
#include"Util/Str.hpp"
#include<basicsecure.h>
#include<stdexcept>

namespace {

std::uint8_t const zero[] ={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

}

namespace Ripemd160 {

bool Hash::valid_string(std::string const& s) {
	return (s.size() == 40)
	    && (Util::Str::ishex(s))
	     ;
}
Hash::Hash(std::string const& s) : pimpl(std::make_shared<Impl>()) {
	auto buf = Util::Str::hexread(s);
	if (buf.size() != 20)
		throw Util::BacktraceException<std::invalid_argument>(
			"Ripemd160::Hash: incorrect size."
		);
	from_buffer(&buf[0]);
}

Hash::operator std::string() const {
	if (!pimpl)
		return "0000000000000000000000000000000000000000";
	return Util::Str::hexdump(pimpl->d, sizeof(pimpl->d));
}
Hash::operator bool() const {
	if (!pimpl)
		return false;
	return !basicsecure_eq(zero, pimpl->d, 20);
}
bool Hash::operator==(Hash const& i) const {
	auto a = pimpl ? pimpl->d : zero;
	auto b = i.pimpl ? i.pimpl->d : zero;
	return basicsecure_eq(a, b, 20);
}

}
