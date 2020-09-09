#undef NDEBUG
#include"DnsSeed/Detail/parse_dig_srv.hpp"
#include<assert.h>

namespace {

auto const text = 
R"DIG(
; <<>> DiG 9.16.1-Ubuntu <<>> @1.0.0.1 lseed.bitcoinstats.com. SRV
; (1 server found)
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 54566
;; flags: qr rd ra; QUERY: 1, ANSWER: 5, AUTHORITY: 0, ADDITIONAL: 1

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 1232
;; QUESTION SECTION:
;lseed.bitcoinstats.com.		IN	SRV

;; ANSWER SECTION:
lseed.bitcoinstats.com.	52	IN	SRV	10 10 9735 ln1q03x4x8wf5fjp4tht74jj9vqj6gcqkfdkdneumfjudsdh528qek2xcr9vc3.lseed.bitcoinstats.com.
lseed.bitcoinstats.com.	52	IN	SRV	10 10 9735 ln1qferzvzn7c7cn2whlrtq9x5h0dwm09hedryz6saeykpfrr7vfqj7wnfn3j2.lseed.bitcoinstats.com.
lseed.bitcoinstats.com.	52	IN	SRV	10 10 9735 ln1qvphmsywntrrhqjcraumvc4y6r8v4z5v593trte429v4hredj7ms5y5qjeu.lseed.bitcoinstats.com.
lseed.bitcoinstats.com.	52	IN	SRV	10 10 9735 ln1qvv3vqfv845efn0xf5g74dmuzl4y6lgvd08lwm8tu2srhjtpw647udzz2p2.lseed.bitcoinstats.com.
lseed.bitcoinstats.com.	52	IN	SRV	10 10 9735 ln1q06gl3dvafdns78ly9sz0um8wc5qtup0a2c35quw5tg5fv9raw9rstvrqk5.lseed.bitcoinstats.com.

;; Query time: 155 msec
;; SERVER: 1.0.0.1#53(1.0.0.1)
;; WHEN: Wed Sep 09 17:36:46 PST 2020
;; MSG SIZE  rcvd: 598

)DIG"
;

auto const reference = std::vector<DnsSeed::Detail::Record>
{ {"03e26a98ee4d1320d5775fab291580969180592db3679e6d32e360dbd147066ca3", 9735, "ln1q03x4x8wf5fjp4tht74jj9vqj6gcqkfdkdneumfjudsdh528qek2xcr9vc3.lseed.bitcoinstats.com."}
, {"0272313053f63d89a9d7f8d6029a977b5db796f968c82d43b92582918fcc4825e7", 9735, "ln1qferzvzn7c7cn2whlrtq9x5h0dwm09hedryz6saeykpfrr7vfqj7wnfn3j2.lseed.bitcoinstats.com."}
, {"03037dc08e9ac63b82581f79b662a4d0ceca8a8ca162b1af3551595b8f2d97b70a", 9735, "ln1qvphmsywntrrhqjcraumvc4y6r8v4z5v593trte429v4hredj7ms5y5qjeu.lseed.bitcoinstats.com."}
, {"031916012c3d6994cde64d11eab77c17ea4d7d0c6bcff76cebe2a03bc96176abee", 9735, "ln1qvv3vqfv845efn0xf5g74dmuzl4y6lgvd08lwm8tu2srhjtpw647udzz2p2.lseed.bitcoinstats.com."}
, {"03f48fc5acea5b3878ff216027f367762805f02feab11a038ea2d144b0a3eb8a38", 9735, "ln1q06gl3dvafdns78ly9sz0um8wc5qtup0a2c35quw5tg5fv9raw9rstvrqk5.lseed.bitcoinstats.com."}
};

}

int main() {
	auto rv = DnsSeed::Detail::parse_dig_srv(text);
	assert(rv.size() == reference.size());
	for (auto i = size_t(0); i < rv.size(); ++i) {
		assert(rv[i].nodeid == reference[i].nodeid);
		assert(rv[i].port == reference[i].port);
		assert(rv[i].hostname == reference[i].hostname);
	}
	return 0;
}
