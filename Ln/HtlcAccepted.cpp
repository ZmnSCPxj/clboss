#include"Ln/HtlcAccepted.hpp"

namespace Ln { namespace HtlcAccepted {

/* data Response = Continue Int64
 *               | Fail Int64 [Int8]
 *               | Resolve Int64 Sha256.Hash
 */
class Response::Impl {
private:
	enum Type
	{ Continue
	, Fail
	, Resolve
	};
	Type type;
	Ln::CommandId id;
	union {
		std::uint8_t fail[sizeof(std::vector<std::uint8_t>)];
		std::uint8_t resolve[sizeof(Ln::Preimage)];
	} d;

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;
	~Impl() {
		if (type == Fail) {
			reinterpret_cast<std::vector<std::uint8_t>*>(
				d.fail
			)->~vector();
		} else if (type == Resolve) {
			reinterpret_cast<Ln::Preimage*>(
				d.resolve
			)->~Preimage();
		}
	}
	explicit
	Impl(Ln::CommandId id_) : type(Continue), id(id_) { }
	explicit
	Impl( Ln::CommandId id_
	    , std::vector<std::uint8_t> message
	    ) : type(Fail), id(id_) {
		new((void*) d.fail) std::vector<std::uint8_t>(
			std::move(message)
		);
	}
	explicit
	Impl( Ln::CommandId id_
	    , Ln::Preimage preimage
	    ) : type(Resolve), id(id_) {
		new((void*) d.resolve) Ln::Preimage(
			std::move(preimage)
		);
	}

	bool is_cont() const { return type == Continue; }
	bool is_fail() const { return type == Fail; }
	bool is_resolve() const { return type == Resolve; }

	Ln::CommandId const& get_id() const { return id; }
	std::vector<std::uint8_t> const& fail_message() const {
		return *reinterpret_cast<std::vector<std::uint8_t> const*>(
			d.fail
		);
	}
	Ln::Preimage const& resolve_preimage() const {
		return *reinterpret_cast<Ln::Preimage const*>(
			d.resolve
		);
	}
};

Response Response::cont(Ln::CommandId id) {
	auto rv = Response();
	rv.pimpl = std::make_shared<Impl>(id);
	return rv;
}
Response Response::fail(Ln::CommandId id, std::vector<std::uint8_t> message) {
	auto rv = Response();
	rv.pimpl = std::make_shared<Impl>(id, std::move(message));
	return rv;
}
Response Response::resolve(Ln::CommandId id, Ln::Preimage preimage) {
	auto rv = Response();
	rv.pimpl = std::make_shared<Impl>(id, std::move(preimage));
	return rv;
}

Response::operator bool() const { return !!pimpl; }

bool Response::is_cont() const { return pimpl->is_cont(); }
bool Response::is_fail() const { return pimpl->is_fail(); }
bool Response::is_resolve() const { return pimpl->is_resolve(); }

Ln::CommandId const& Response::id() const { return pimpl->get_id(); }
std::vector<std::uint8_t> const& Response::fail_message() const {
	return pimpl->fail_message();
}
Ln::Preimage const& Response::resolve_preimage() const {
	return pimpl->resolve_preimage();
}

}}
