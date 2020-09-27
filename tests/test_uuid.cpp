#undef NDEBUG
#include"Uuid.hpp"
#include<assert.h>

int main() {
	auto a = Uuid();
	auto b = Uuid();
	assert(a == b);
	assert(Uuid() == b);
	assert(a == Uuid());
	assert(!a);

	a = Uuid::random();
	assert(a); /* With very high probability, at least.  */
	assert(a != b);
	b = a;
	assert(a == b);
	assert(std::hash<Uuid>()(a) == std::hash<Uuid>()(b));

	b = (Uuid)(std::string)a;
	assert(a == b);
	assert(std::hash<Uuid>()(a) == std::hash<Uuid>()(b));

	a = Uuid("00112233445566778899aabbccddeeff");
	b = Uuid("00112233445566778899aabbccddeeff");
	assert(a == b);
	assert(a);
	assert(b);
	assert(std::hash<Uuid>()(a) == std::hash<Uuid>()(b));
	assert(std::string(a) == "00112233445566778899aabbccddeeff");

	b = Uuid("00112233445566778899aabbccddeefe");
	assert(a != b);

	return 0;
}

