#ifndef SECP256K1_DETAIL_CONTEXT_HPP
#define SECP256K1_DETAIL_CONTEXT_HPP

#include<memory>

extern "C" {
struct secp256k1_context_struct;
}

namespace Secp256k1 {
namespace Detail {

/* A shared pointer is mildly overweight for this purpose,
 * but the struct definition is hidden inside libsecp256k1
 * (so cannot instantiate directly), a plain pointer will
 * not have a destructor, a unique_ptr would need to expose
 * the deletor type here as well.
 * A shared pointer has the nice property of also hiding
 * the deletor type (it is hidden in the management object
 * maintained by shared pointers).
 */
extern std::shared_ptr<secp256k1_context_struct> const context;

}
}

#endif /* SECP256K1_DETAIL_CONTEXT_HPP */
