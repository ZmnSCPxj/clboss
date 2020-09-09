#undef NDEBUG
#include"DnsSeed/Detail/decode_bech32_node.hpp"
#include"Util/Str.hpp"
#include<assert.h>

namespace {

/* Why o why does not BOLT#10 contain test vectors?  */
auto const tests = std::vector<std::pair<std::string, std::string>>
{ { "ln1qgmmkxdk5n4a2epe4g953alwmch3668s4lptsct2h3rtqvjd72t0x2r8ynu"
  , "0237bb19b6a4ebd56439aa0b48f7eede2f1d68f0afc2b8616abc46b0324df296f3"
  }
, { "ln1qghm7eksc62cjj74m6rwx4my7jysln05aathk0utcf0064d57gswx3hzaw9"
  , "022fbf66d0c695894bd5de86e35764f4890fcdf4ef577b3f8bc25efd55b4f220e3"
  }
, { "ln1qfsaugkeyxqgrsk3hp8uw0u4t65umk69v6h2knflk7gfde5w3hf6j02k8ha"
  , "0261de22d9218081c2d1b84fc73f955ea9cddb4566aeab4d3fb79096e68e8dd3a9"
  }
, { "ln1qf9ejxwhsf5lz8s8umh93mp623f5066faaty25c5lrvtqms968mtjx8dyls"
  , "024b9919d78269f11e07e6ee58ec3a545347eb49ef56455314f8d8b06e05d1f6b9"
  }
};

}

int main() {
	for (auto const& test : tests) {
		auto const& inp = test.first;
		auto const& exp = test.second;
		auto val = DnsSeed::Detail::decode_bech32_node(inp);
		auto out = Util::Str::hexdump(&val[0], val.size());
		assert(out == exp);
	}
	return 0;
}
