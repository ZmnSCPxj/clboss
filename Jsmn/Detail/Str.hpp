#ifndef JSMN_DETAIL_STR_HPP
#define JSMN_DETAIL_STR_HPP

#include<string>

namespace Jsmn { namespace Detail { namespace Str {

/* Process a "raw" string and insert escape codes.  */
std::string to_escaped(std::string const&);
/* Process an escaped string and return "raw" string.  */
std::string from_escaped(std::string const&);

/* Parse and serialize doubles, in a JSON-compatible way.  */
double to_double(std::string const&);
std::string from_double(double);

}}}

#endif /* !defined(JSMN_DETAIL_STR_HPP) */
