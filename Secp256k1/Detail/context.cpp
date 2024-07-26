#include<secp256k1.h>
#include<stdexcept>
#include<string>
#include"Secp256k1/Detail/context.hpp"
#include"Util/BacktraceException.hpp"

namespace {

/* Called due to illegal inputs to the library.  */
void illegal_callback(const char* msg, void*) {
	throw Util::BacktraceException<std::invalid_argument>(std::string("SECP256K1: ") + msg);
}

std::shared_ptr<secp256k1_context_struct> create_secp256k1_context() {
	auto ctx = secp256k1_context_create( SECP256K1_CONTEXT_SIGN
					   | SECP256K1_CONTEXT_VERIFY
					   );
	auto ret = std::shared_ptr<secp256k1_context_struct>( ctx
							    , &secp256k1_context_destroy
							    );
	secp256k1_context_set_illegal_callback( ret.get()
					      , &illegal_callback
					      , nullptr
					      );
	return ret;
}

}

namespace Secp256k1 {
namespace Detail {
std::shared_ptr<secp256k1_context_struct> const context
	= create_secp256k1_context();
}
}
