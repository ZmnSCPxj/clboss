#undef NDEBUG
#include"Sha256/Hash.hpp"
#include<assert.h>
#include<unordered_set>

int main() {
	auto a = Sha256::Hash();
	auto b = Sha256::Hash();
	assert(a == b);
	b = Sha256::Hash("0123456789012345678901234567890123456789012345678901234567890123");
	assert(a != b);
	assert(Sha256::Hash::valid_string("0123456789012345678901234567890123456789012345678901234567890123"));

	a = Sha256::Hash("3210987654321098765432109876543210987654321098765432109876543210");
	assert(a != b);

	auto bag = std::unordered_set<Sha256::Hash>();
	bag.insert(Sha256::Hash("0123456789012345678901234567890123456789012345678901234567890123"));
	assert(bag.find(b) != bag.end());
	bag.insert(b);
	assert(bag.size() == 1);

	assert(std::string(b) == "0123456789012345678901234567890123456789012345678901234567890123");

	assert(a);
	assert(b);
	a = Sha256::Hash();
	assert(!a);
	b = Sha256::Hash("0000000000000000000000000000000000000000000000000000000000000000");
	assert(!b);

	return 0;
}
