#ifndef S_DETAIL_SIGNALBASE_HPP
#define S_DETAIL_SIGNALBASE_HPP

namespace S { namespace Detail {

/* Common base class for all S::Detail::Signal<>.  */
struct SignalBase {
	virtual ~SignalBase() { }
};

}}

#endif /* !defined(S_DETAIL_SIGNALBASE_HPP) */
