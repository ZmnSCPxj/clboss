#ifndef JSMN_DETAIL_ENDADVANCER_HPP
#define JSMN_DETAIL_ENDADVANCER_HPP

#include<locale>
#include<utility>

namespace Jsmn { namespace Detail {

class SourceReader;

/* Scans through a source of characters, scanning
 * until it reaches a potential end-of-datum
 * character.
 * A character is end-of-datum if it is the last
 * char of a datum.
 */
class EndAdvancer {
private:
	std::locale loc;
	SourceReader& sr;
	bool was_white;

public:
	explicit
	EndAdvancer(SourceReader& sr_)
		: loc("C")
		, sr(sr_)
		, was_white(true)
		{ }

	void scan();
};

/* Abstract base class.  */
class SourceReader {
public:
	/* Should be std::optional<char>.  */
	virtual std::pair<bool, char> read() =0;
};

}}
#endif /* !defined(JSMN_DETAIL_ENDADVANCER_HPP) */
