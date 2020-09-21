#include<memory>
#include<secp256k1.h>
#include"Secp256k1/G.hpp"
#include"Secp256k1/PubKey.hpp"

/* FIXME: Load G as a constant directly.  */
/* G = 0279BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798 */
namespace {

std::uint8_t g[33] = {
	0x02,
	0x79, 0xBE, 0x66, 0x7E,
	0xF9, 0xDC, 0xBB, 0xAC,
	0x55, 0xA0, 0x62, 0x95,
	0xCE, 0x87, 0x0B, 0x07,
	0x02, 0x9B, 0xFC, 0xDB,
	0x2D, 0xCE, 0x28, 0xD9,
	0x59, 0xF2, 0x81, 0x5B,
	0x16, 0xF8, 0x17, 0x98
};

Secp256k1::PubKey make_g() {
	/* Create a temporary context for ourself; we cannot be
	 * certain that Secp256k1::Detail::context has been
	 * properly initialized yet!
	 */
	auto ctx = secp256k1_context_create( SECP256K1_CONTEXT_SIGN
					   | SECP256K1_CONTEXT_VERIFY
					   );
	/* Auto-frees the context later.  */
	auto handler = std::shared_ptr<secp256k1_context_struct>( ctx
								, &secp256k1_context_destroy
								);
	return Secp256k1::PubKey::from_buffer_with_context(handler.get(), g);
}

}

namespace Secp256k1 {

PubKey G = make_g();

}
