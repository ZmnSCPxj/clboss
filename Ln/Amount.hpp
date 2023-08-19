#ifndef LN_AMOUNT_HPP
#define LN_AMOUNT_HPP

#include"Jsmn/Object.hpp"
#include<cstdint>
#include<iostream>
#include<string>

namespace Ln {

/** class Ln::Amount
 *
 * @brief represents some amount of Bitcoins.
 */
class Amount {
private:
	/* In millisatoshi.  */
	std::uint64_t v;

public:
	Amount() : v(0) { }
	Amount(Amount const&) =default;
	Amount& operator=(Amount const&) =default;
	~Amount() =default;

	explicit
	Amount(std::string const&);
	explicit
	operator std::string() const;
	/* Return false if Amount() would throw given this
	 * string.  */
	static
	bool valid_string(std::string const&);

	static
	Amount object(Jsmn::Object const&);
	/* Return false if Amount() would throw given this
	 * object. */
	static
	bool valid_object(Jsmn::Object const&);

	static
	Amount btc(double v) {
		auto ret = Amount();
		ret.v = v * (100000000.0 * 1000.0);
		return ret;
	}
	double to_btc() const {
		return ((double) v) / (100000000.0 * 1000.0);
	}

	static
	Amount sat(std::uint64_t v) {
		auto ret = Amount();
		ret.v = v * 1000;
		return ret;
	}
	std::uint64_t to_sat() const {
		return (v + 999) / 1000;
	}

	static
	Amount msat(std::uint64_t v) {
		auto ret = Amount();
		ret.v = v;
		return ret;
	}
	std::uint64_t to_msat() const {
		return v;
	}

	/* Saturates.  */
	Amount& operator+=(Amount const& i) {
		v += i.v;
		if (v < i.v)
			v = UINT64_MAX;
		return *this;
	}
	Amount operator+(Amount const& i) const {
		return Amount(*this) += i;
	}
	Amount& operator-=(Amount const& i) {
		if (i.v > v)
			v = 0;
		else
			v -= i.v;
		return *this;
	}
	Amount operator-(Amount const& i) const {
		return Amount(*this) -= i;
	}
	Amount& operator*=(double i) {
		auto dv = double(v);
		dv *= i;
		if (dv < 0)
			v = 0;
		else if (dv > double(UINT64_MAX))
			v = UINT64_MAX;
		else
			v = dv;
		return *this;
	}
	Amount operator*(double i) const {
		return Amount(*this) *= i;
	}
	Amount& operator/=(double i) {
		auto dv = double(v);
		dv /= i;
		if (dv < 0)
			v = 0;
		else if (dv > double(UINT64_MAX))
			v = UINT64_MAX;
		else
			v = dv;
		return *this;
	}
	Amount operator/(double i) const {
		return Amount(*this) /= i;
	}
	double operator/(Amount const& i) const {
		return ((double)v) / ((double)i.v);
	}

	bool operator<(Amount const& o) const {
		return v < o.v;
	}
	bool operator>(Amount const& o) const {
		return o < (*this);
	}
	bool operator<=(Amount const& o) const {
		return !(*this > o);
	}
	bool operator>=(Amount const& o) const {
		return o <= (*this);
	}
	bool operator==(Amount const& o) const {
		return v == o.v;
	}
	bool operator!=(Amount const& o) const {
		return !(*this == o);
	}
};

inline
Amount operator*(double i, Amount v) {
	return v * i;
}

inline
std::ostream& operator<<(std::ostream& os, Amount const& v) {
	return os << std::string(v);
}
inline
std::istream& operator>>(std::istream& is, Amount& v) {
	auto s = std::string();
	is >> s;
	v = Amount(s);
	return is;
}

}

#endif /* !defined(LN_AMOUNT_HPP) */
