#undef NDEBUG
#include"Ln/Scid.hpp"
#include<assert.h>

int main() {
	auto a = Ln::Scid("1x1x1");
	auto b = Ln::Scid("1x1x2");
	assert(a != b);
	b = a;
	assert(a == b);

	assert(!Ln::Scid::valid_string("garbage"));
	assert(Ln::Scid::valid_string("85824x235x3"));

	a = Ln::Scid("94x2x3");
	b = Ln::Scid("0x2x3");
	assert(b < a);
	assert(a > b);
	assert(b <= a);
	assert(a >= b);
	assert(!(a <= b));
	assert(!(b >= a));
	b = a;
	assert(b <= a);
	assert(a >= b);

	/* Out of range.  */
	assert(!Ln::Scid::valid_string("1x1x65537"));
	assert(!Ln::Scid::valid_string("1x16777777x1"));
	assert(!Ln::Scid::valid_string("16777777x1x1"));

	/* Strings.  */
	assert(std::string(Ln::Scid("5x4x1")) == "5x4x1");

	return 0;
}
