#undef NDEBUG
#include"Ln/HtlcAccepted.hpp"
#include"Util/Str.hpp"
#include<assert.h>

int main() {
	auto c = Ln::HtlcAccepted::Response::cont(42);
	assert(c);
	assert(!!c);
	assert(c.is_cont());
	assert(!c.is_fail());
	assert(!c.is_resolve());
	assert(c.id() == 42);

	auto f = Ln::HtlcAccepted::Response::fail(
		42, Util::Str::hexread("4242")
	);
	assert(f);
	assert(!!f);
	assert(!f.is_cont());
	assert(f.is_fail());
	assert(!f.is_resolve());
	assert(f.id() == 42);
	assert(f.fail_message() == Util::Str::hexread("4242"));

	auto r = Ln::HtlcAccepted::Response::resolve(
		42,
		Ln::Preimage("4242424242424242424242424242424242424242424242424242424242424242")
	);
	assert(r);
	assert(!!r);
	assert(!r.is_cont());
	assert(!r.is_fail());
	assert(r.is_resolve());
	assert(r.id() == 42);
	assert(r.resolve_preimage() == Ln::Preimage("4242424242424242424242424242424242424242424242424242424242424242"));

	return 0;
}
