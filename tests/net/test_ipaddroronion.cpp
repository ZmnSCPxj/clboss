#undef NDEBUG
#include"Net/IPAddr.hpp"
#include"Net/IPAddrOrOnion.hpp"
#include"Util/stringify.hpp"
#include<assert.h>

int main() {
	using Net::IPAddr;
	using Net::IPAddrOrOnion;

	assert(std::string(IPAddrOrOnion()) == "127.0.0.1");
	assert(IPAddrOrOnion() == IPAddrOrOnion("127.0.0.1"));
	assert(IPAddrOrOnion("2607:f0d0:1002:51::4") != IPAddrOrOnion("192.168.1.1"));
	assert(IPAddrOrOnion("192.168.1.1").is_ip_addr());
	assert(IPAddrOrOnion("2607:f0d0:1002:51::4").is_ip_addr());
	assert(IPAddrOrOnion("ajnvpgl6prmkb7yktvue6im5wiedlz2w32uhcwaamdiecdrfpwwgnlqd.onion").is_onion());

	{
		auto flag = false;
		try {
			(void) IPAddrOrOnion("neither IP nor onion");
		} catch (Net::IPAddrInvalid const& _) {
			flag = true;
		}
		assert(flag);
	}

	assert( IPAddrOrOnion("ajnvpgl6prmkb7yktvue6im5wiedlz2w32uhcwaamdiecdrfpwwgnlqd.onion")
	     == IPAddrOrOnion::from_onion("ajnvpgl6prmkb7yktvue6im5wiedlz2w32uhcwaamdiecdrfpwwgnlqd.onion")
	      );
	assert( IPAddrOrOnion("192.168.1.1")
	     == IPAddrOrOnion::from_ip_addr(IPAddr::v4("192.168.1.1"))
	      );
	assert( IPAddrOrOnion("2607:f0d0:1002:51::4")
	     == IPAddrOrOnion::from_ip_addr(IPAddr::v6("2607:f0d0:1002:51::4"))
	      );

	{
		auto flag = false;
		try {
			(void) IPAddrOrOnion::from_onion("not a dot onion");
		} catch (Net::IPAddrInvalid const& _) {
			flag = true;
		}
		assert(flag);
	}
	{
		auto flag = false;
		try {
			(void) IPAddrOrOnion::from_ip_addr_v4("2607:f0d0:1002:51::4");
		} catch (Net::IPAddrInvalid const& _) {
			flag = true;
		}
		assert(flag);
	}
	{
		auto flag = false;
		try {
			(void) IPAddrOrOnion::from_ip_addr_v6("192.168.1.1");
		} catch (Net::IPAddrInvalid const& _) {
			flag = true;
		}
		assert(flag);
	}

	{
		auto ip = IPAddr();
		IPAddrOrOnion("192.168.1.1").to_ip_addr(ip);
		assert(ip == IPAddr::v4("192.168.1.1"));
	}
	{
		auto onion = std::string();
		IPAddrOrOnion("ajnvpgl6prmkb7yktvue6im5wiedlz2w32uhcwaamdiecdrfpwwgnlqd.onion").to_onion(onion);
		assert(onion == "ajnvpgl6prmkb7yktvue6im5wiedlz2w32uhcwaamdiecdrfpwwgnlqd.onion");
	}

	return 0;
}
