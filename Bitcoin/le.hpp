#ifndef BITCOIN_LE_HPP
#define BITCOIN_LE_HPP

#include"Ln/Amount.hpp"
#include<cstdint>
#include<iostream>

namespace Bitcoin { namespace Detail { class Le16; }}
namespace Bitcoin { namespace Detail { class Le16Const; }}
namespace Bitcoin { namespace Detail { class Le32; }}
namespace Bitcoin { namespace Detail { class Le32Const; }}
namespace Bitcoin { namespace Detail { class Le64; }}
namespace Bitcoin { namespace Detail { class Le64Const; }}
namespace Bitcoin { namespace Detail { class LeAmount; }}
namespace Bitcoin { namespace Detail { class LeAmountConst; }}

namespace Bitcoin {

/** Bitcoin::le
 *
 * @brief wraps a uint64, int64, uint32, int32, or
 * `Ln::Amount` so it is encoded in little-endian
 * form.
 *
 * @desc intended use is:
 *
 *     std::cout << Bitcoin::le(expr);
 *     std::cin >> Bitcoin::le(var);
 */
Detail::Le16 le(std::uint16_t& v);
Detail::Le16Const le(std::uint16_t const& v);
Detail::Le16 le(std::int16_t& v);
Detail::Le16Const le(std::int16_t const& v);
Detail::Le32 le(std::uint32_t& v);
Detail::Le32Const le(std::uint32_t const& v);
Detail::Le32 le(std::int32_t& v);
Detail::Le32Const le(std::int32_t const& v);
Detail::Le64 le(std::uint64_t& v);
Detail::Le64Const le(std::uint64_t const& v);
Detail::Le64 le(std::int64_t& v);
Detail::Le64Const le(std::int64_t const& v);
Detail::LeAmount le(Ln::Amount& v);
Detail::LeAmountConst le(Ln::Amount const& v);

}

std::ostream& operator<<(std::ostream&, Bitcoin::Detail::Le16Const);
std::ostream& operator<<(std::ostream&, Bitcoin::Detail::Le32Const);
std::ostream& operator<<(std::ostream&, Bitcoin::Detail::Le64Const);
std::ostream& operator<<(std::ostream&, Bitcoin::Detail::LeAmountConst);
std::istream& operator>>(std::istream&, Bitcoin::Detail::Le16);
std::istream& operator>>(std::istream&, Bitcoin::Detail::Le32);
std::istream& operator>>(std::istream&, Bitcoin::Detail::Le64);
std::istream& operator>>(std::istream&, Bitcoin::Detail::LeAmount);

namespace Bitcoin { namespace Detail {

class Le16Const {
private:
	std::uint16_t v;

	friend
	std::ostream& ::operator<<(std::ostream&, Bitcoin::Detail::Le16Const);

public:
	Le16Const(std::uint16_t const& v_) : v(v_) { }
};

class Le16 {
private:
	std::uint16_t& v;

	friend
	std::istream& ::operator>>(std::istream&, Bitcoin::Detail::Le16);

public:
	Le16(std::uint16_t& v_) : v(v_) { }
	operator Le16Const() const { return Le16Const(v); }
};

class Le32Const {
private:
	std::uint32_t v;

	friend
	std::ostream& ::operator<<(std::ostream&, Bitcoin::Detail::Le32Const);

public:
	Le32Const(std::uint32_t const& v_) : v(v_) { }
};

class Le32 {
private:
	std::uint32_t& v;

	friend
	std::istream& ::operator>>(std::istream&, Bitcoin::Detail::Le32);

public:
	Le32(std::uint32_t& v_) : v(v_) { }
	operator Le32Const() const { return Le32Const(v); }
};

class Le64Const {
private:
	std::uint64_t v;

	friend
	std::ostream& ::operator<<(std::ostream&, Bitcoin::Detail::Le64Const);

public:
	Le64Const(std::uint64_t const& v_) : v(v_) { }
};

class Le64 {
private:
	std::uint64_t& v;

	friend
	std::istream& ::operator>>(std::istream&, Bitcoin::Detail::Le64);

public:
	Le64(std::uint64_t& v_) : v(v_) { }
	operator Le64Const() const { return Le64Const(v); }
};

class LeAmountConst {
private:
	Ln::Amount v;

	friend
	std::ostream& ::operator<<(std::ostream&, Bitcoin::Detail::LeAmountConst);

public:
	LeAmountConst(Ln::Amount const& v_) : v(v_) { }
};

class LeAmount {
private:
	Ln::Amount& v;

	friend
	std::istream& ::operator>>(std::istream&, Bitcoin::Detail::LeAmount);

public:
	LeAmount(Ln::Amount& v_) : v(v_) { }
	operator LeAmountConst() const { return LeAmountConst(v); }
};

}}

#endif /* !defined(BITCOIN_LE_HPP) */
