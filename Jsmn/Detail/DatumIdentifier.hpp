#ifndef JSMN_DETAIL_DATUMIDENTIFIER_HPP
#define JSMN_DETAIL_DATUMIDENTIFIER_HPP

namespace Jsmn { namespace Detail {

/** class Jsmn::Detail::DatumIdentifier
 *
 * @brief Fed one character at a time, determines if
 * this is a potential end of a datum or not.
 */
class DatumIdentifier {
private:
	unsigned int nested;
	bool in_string;
	bool in_string_backslash;
	bool finished_datum;
public:
	DatumIdentifier() {
		nested = 0;
		in_string = false;
		in_string_backslash = false;
		finished_datum = true; 
	}

	/* Return true if end-of-datum.  */
	bool feed(char c) noexcept {
		if (in_string_backslash) {
			in_string_backslash = false;
			return false;
		}
		if (in_string) {
			if (c == '\\') {
				in_string_backslash = true;
				return false;
			}
			if (c == '"') {
				in_string = false;
				return (finished_datum = (nested == 0));
			}
			return false;
		}

		if (c == '"') {
			in_string = true;
			finished_datum = false;
			return false;
		}
		if (c == '{' || c == '[') {
			++nested;
			finished_datum = false;
			return false;
		}
		if (c == '}' || c == ']') {
			if (nested != 0)
				--nested;
			return (finished_datum = (nested == 0));
		}

		/* Whitespace ends datum only if there *is*
		 * an actual datum.
		 */
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v') {
			if (finished_datum) {
				return false;
			}
			return (finished_datum = (nested == 0));
		} else {
			finished_datum = false;
			return false;
		}
	}
};

}}

#endif /* JSMN_DETAIL_DATUMIDENTIFIER_HPP */
