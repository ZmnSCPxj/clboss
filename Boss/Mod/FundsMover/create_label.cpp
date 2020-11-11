#include"Boss/Mod/FundsMover/create_label.hpp"
#include"Sha256/Hash.hpp"
#include"Util/Str.hpp"

namespace {

auto const label_prefix = std::string("CLBOSS FundsMover payment, "
				      "this should automatically "
				      "get deleted. Hash: ");

}

namespace Boss { namespace Mod { namespace FundsMover {

std::string create_label(Sha256::Hash const& payment_hash) {
	return label_prefix + std::string(payment_hash);
}

bool is_our_label(std::string const& label) {
	if (label.size() != label_prefix.length() + 64)
		return false;
	if ( std::string(label.begin(), label.begin() + label_prefix.length())
	  != label_prefix
	   )
		return false;
	auto hash = std::string( label.begin() + label_prefix.length()
			       , label.end()
			       );
	return Util::Str::ishex(hash);
}

}}}
