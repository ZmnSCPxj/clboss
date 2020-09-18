#undef NDEBUG
#include"Sha256/Hash.hpp"
#include"Sha256/Hasher.hpp"
#include"Sha256/fun.hpp"
#include<assert.h>

int main() {
	/* From sha256sum.  */
	assert( Sha256::fun("hello", 5)
	     == Sha256::Hash("2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824")
	      );

	auto hasher = Sha256::Hasher();
	hasher.feed("hel", 3);
	hasher.feed("lo", 2);
	assert( hasher.get()
	     == Sha256::Hash("2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824")
	      );
	assert( std::move(hasher).finalize()
	     == Sha256::Hash("2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824")
	      );

	return 0;
}
