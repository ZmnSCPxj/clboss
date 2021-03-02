#include"Util/Str.hpp"
#include"Util/make_unique.hpp"
#include"Uuid.hpp"
#include<algorithm>
#include<basicsecure.h>
#include<stdexcept>

namespace {

std::uint8_t const zero[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

}

Uuid Uuid::random() {
	auto rv = Uuid();
	rv.pimpl = Util::make_unique<Impl>();
	BASICSECURE_RAND(rv.pimpl->data, sizeof(rv.pimpl->data));
	return rv;
}

bool Uuid::operator==(Uuid const& o) const {
	auto a = pimpl ? pimpl->data : zero;
	auto b = o.pimpl ? o.pimpl->data : zero;
	return basicsecure_eq(a, b, 16);
}

Uuid::operator bool() const {
	if (!pimpl)
		return false;
	return !basicsecure_eq(pimpl->data, zero, 16);
}

Uuid::operator std::string() const {
	if (!pimpl)
		return "00000000000000000000000000000000";
	return Util::Str::hexdump(pimpl->data, 16);
}
Uuid::Uuid(std::string const& s) {
	pimpl = Util::make_unique<Impl>();
	auto buf = Util::Str::hexread(s);
	if (buf.size() != 16)
		throw std::invalid_argument("Uuid: invalid input string.");
	std::copy(buf.begin(), buf.end(), pimpl->data);
}
bool Uuid::valid_string(std::string const& s) {
	return Util::Str::ishex(s) && s.size() == 32;
}
