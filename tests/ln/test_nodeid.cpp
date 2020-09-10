#undef NDEBUG
#include"Ln/NodeId.hpp"
#include<assert.h>
#include<sstream>

#include<iostream>

int main() {
	auto a = Ln::NodeId();
	auto b = Ln::NodeId();
	assert(!a);
	assert(!b);
	assert(a == b);
	assert(a == Ln::NodeId("000000000000000000000000000000000000000000000000000000000000000000"));
	assert(std::string(a) == "000000000000000000000000000000000000000000000000000000000000000000");

	assert(a != Ln::NodeId("02a209432db157f414e668dbce5ee0f986dedd7b9ecbbdb4cf9a47215e85f70d5c"));
	assert(Ln::NodeId("03dfafd14dbaeda53f23e01e83af79452a2058a40291daa76552913b503c501e22") != b);

	a = Ln::NodeId("03f2d334ab70d50623c889400941dc80874f38498e7d09029af0f701d7089aa516");
	assert(a);
	assert(a != b);
	b = a;
	assert(b);
	assert(a == b);
	assert(std::string(b) == "03f2d334ab70d50623c889400941dc80874f38498e7d09029af0f701d7089aa516");

	a = Ln::NodeId("0247719ca61fa34ce383634a30b9cde6a2b396a8a6e2974dbbd4ebbe93e093ad2c");
	b = Ln::NodeId("0247719ca61fa34ce383634a30b9cde6a2b396a8a6e2974dbbd4ebbe93e093ad2c");
	assert(a == b);

	auto ss = std::stringstream();
	a = Ln::NodeId("02d37a83a9cc83364cb92a4c0df3f76d353b7f4fb0b20ec06485e15c2c51fe7a4f");
	ss << a;
	ss >> std::ws >> b;
	assert(a == b);

	ss = std::stringstream();
	a = Ln::NodeId("039867f8c14d6c95ce234ab11afd7c450c049ecf3258633f5ced5160716816eab6");
	ss << a;
	auto str = std::string();
	ss >> std::ws >> str;
	assert(str == "039867f8c14d6c95ce234ab11afd7c450c049ecf3258633f5ced5160716816eab6");

	ss = std::stringstream();
	ss << "037dda58c0e1b81237f1fecf4cea99e9aaa5918fdbd0fb87042219f0b008ad10c1";
	ss >> std::ws >> a;
	assert(a == Ln::NodeId("037dda58c0e1b81237f1fecf4cea99e9aaa5918fdbd0fb87042219f0b008ad10c1"));

	return 0;
}
