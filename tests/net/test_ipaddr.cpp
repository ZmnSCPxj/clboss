#undef NDEBUG
#include"Net/IPAddr.hpp"
#include"Util/stringify.hpp"
#include<assert.h>

int main() {
	using Net::IPAddr;
	assert(IPAddr() == IPAddr::v4("127.0.0.1"));
	assert(std::string(IPAddr()) == "127.0.0.1");
	assert(std::string(IPAddr::v6("::1")) == "::1");
	assert(IPAddr::v4("127.0.0.1") != IPAddr::v4("192.168.1.1"));

	assert(IPAddr::v4("192.168.1.1").is_ipv4());
	assert(!IPAddr::v4("192.168.1.1").is_ipv6());
	assert(!IPAddr::v6("2607:f0d0:1002:0051:0000:0000:0000:0004").is_ipv4());
	assert(IPAddr::v6("2607:f0d0:1002:0051:0000:0000:0000:0004").is_ipv6());

	assert(IPAddr::v6("2607:f0d0:1002:0051:0000:0000:0000:0004") == IPAddr::v6("2607:f0d0:1002:51::4"));

	assert(Util::stringify(IPAddr()) == "127.0.0.1");

	{
		auto ip = IPAddr::v4("192.168.1.2");
		auto& raw = ip.raw_data();
		assert(raw.size() == 4);
		assert(raw[0] == 192);
		assert(raw[1] == 168);
		assert(raw[2] == 1);
		assert(raw[3] == 2);
	}
	{
		auto ip = IPAddr::v6("2607:f0d0:1002:0051:0000:0000:0000:0004");
		auto& raw = ip.raw_data();
		assert(raw.size() == 16);
		assert(raw[0] == 0x26);
		assert(raw[1] == 0x07);
		assert(raw[2] == 0xf0);
		assert(raw[3] == 0xd0);
		assert(raw[4] == 0x10);
		assert(raw[5] == 0x02);
		assert(raw[6] == 0x00);
		assert(raw[7] == 0x51);
		assert(raw[8] == 0x00);
		assert(raw[9] == 0x00);
		assert(raw[10] == 0x00);
		assert(raw[11] == 0x00);
		assert(raw[12] == 0x00);
		assert(raw[13] == 0x00);
		assert(raw[14] == 0x00);
		assert(raw[15] == 0x04);
	}

	bool flag = false;
	try {
		(void) IPAddr::v4("Invalid address");
	} catch (Net::IPAddrInvalid& _) {
		flag = true;
	}
	assert(flag);

	flag = false;
	try {
		(void) IPAddr::v6("Invalid address");
	} catch (Net::IPAddrInvalid& _) {
		flag = true;
	}
	assert(flag);
}
