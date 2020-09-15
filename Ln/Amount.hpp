#ifndef LN_AMOUNT_HPP
#define LN_AMOUNT_HPP

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
	Amount btc(double v) {
		auto ret = Amount();
		ret.v = v * (100000000.0 * 1000.0);
		return ret;
	}
	static
	Amount sat(std::uint64_t v) {
		auto ret = Amount();
		ret.v = v * 1000;
		return ret;
	}
	static
	Amount msat(std::uint64_t v) {
		auto ret = Amount();
		ret.v = v;
		return ret;
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
		auto tmp = v;
		v *= i;
		if (v / i != tmp)
			v = UINT64_MAX;
		return *this;
	}
	Amount operator*(double i) const {
		return Amount(*this) *= i;
	}
	Amount& operator/=(double i) {
		auto tmp = v;
		v /= i;
		if (v * i != tmp)
			v = 0;
		return *this;
	}
	Amount operator/(double i) const {
		return Amount(*this) /= i;
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
