#ifndef LN_NODEID_HPP
#define LN_NODEID_HPP

#include<cstdint>
#include<iostream>
#include<memory>
#include<string>

namespace Ln {

/** class Ln::NodeId
 *
 * @brief object to represent the public key of a
 * node, i.e. the node ID.
 */
class NodeId {
private:
	struct Impl {
		std::uint8_t raw[33];
	};
	std::shared_ptr<Impl const> pimpl;

public:
	NodeId() =default;
	NodeId(NodeId&& o) : pimpl(std::move(o.pimpl)) { }
	NodeId(NodeId const&) =default;
	explicit
	NodeId(std::string const&);
	~NodeId() =default;

	static bool valid_string(std::string const&);

	NodeId& operator=(NodeId const&) =default;
	NodeId& operator=(NodeId&& o) {
		auto tmp = std::move(o);
		std::swap(pimpl, tmp.pimpl);
		return *this;
	}

	explicit
	operator std::string() const;

	explicit
	operator bool() const { return !!pimpl; }
	bool operator!() const { return !bool(*this); }

private:
	bool equality_check(NodeId const& o) const;
public:
	bool operator==(NodeId const& o) const {
		if (o.pimpl == pimpl)
			return true;
		return equality_check(o);
	}
	bool operator!=(NodeId const& o) const {
		return !(*this == o);
	}

private:
	bool less_check(NodeId const& o) const;
public:
	bool operator<(NodeId const& o) const {
		if (o.pimpl == pimpl)
			return false;
		return less_check(o);
	}
	bool operator>(NodeId const& o) const {
		return (o < *this);
	}
	bool operator<=(NodeId const& o) const {
		return !(*this > o);
	}
	bool operator>=(NodeId const& o) const {
		return (o <= *this);
	}
};

std::istream& operator>>(std::istream&, NodeId&);
std::ostream& operator<<(std::ostream&, NodeId const&);

}

#endif /* LN_NODEID_HPP */
