#undef NDEBUG
#include"Ln/Amount.hpp"
#include<assert.h>
#include<sstream>
#include<stdexcept>

int main() {
	assert(Ln::Amount::msat(42) == Ln::Amount("42msat"));
	assert(Ln::Amount::btc(0.005) == Ln::Amount::sat(500000));
	assert(Ln::Amount::sat(42) == Ln::Amount::msat(42000));

	auto a = Ln::Amount::sat(547);
	auto b = Ln::Amount::msat(99999);
	assert(a > b);
	b = a;
	assert(a == b);
	assert(a + b >= b);
	assert(a + b == b + a);
	assert(a - b == Ln::Amount("0msat"));

	{
		auto os = std::ostringstream();
		os << a + b;
		assert(os.str() == "1094000msat");
	}

	auto flag = false;
	try {
		auto tmp = Ln::Amount("garbage");
		(void) tmp;
		assert(false);
	} catch (std::invalid_argument const& _) {
		flag = true;
	}
	assert(flag);
	flag = false;
	try {
		/* Printer go brr... */
		auto tmp = Ln::Amount("999999999999999999999999999999999msat");
		(void) tmp;
		assert(false);
	} catch (std::invalid_argument const& _) {
		flag = true;
	}
	assert(flag);

	assert(Ln::Amount::sat(1) * 0.5 == Ln::Amount::msat(500));
	assert(Ln::Amount::sat(1) / 0.5 == Ln::Amount::sat(2));
	assert(2 * Ln::Amount::btc(1) == Ln::Amount("200000000000msat"));

	assert( Ln::Amount::sat(1) - 0.25 * Ln::Amount::sat(1)
	     == Ln::Amount::sat(1) * 0.75
	      );

	return 0;
}
