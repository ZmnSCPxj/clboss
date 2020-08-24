#ifndef S_DETAIL_SIGNAL_HPP
#define S_DETAIL_SIGNAL_HPP

#include<Ev/Io.hpp>
#include<Ev/yield.hpp>
#include<S/Detail/SignalBase.hpp>
#include<Util/make_unique.hpp>
#include<functional>
#include<memory>

namespace S { namespace Detail {

/* Registers callbacks for a particular type a, and
 * broadcasts to all callbacks.  */
template<typename a>
class Signal : public SignalBase {
private:
	/* Singly-linked list, for stable, low-overhead storage.  */
	struct Node {
		std::function<Ev::Io<void>(a const&)> callback;
		std::unique_ptr<Node> next;
	};
	std::unique_ptr<Node> first;
	Node* last;

public:
	Signal() : first(), last(nullptr) { }

private:
	static
	Ev::Io<void> raise_loop( std::shared_ptr<a> pvalue
			       , Node *it
			       ) {
		if (!it)
			return Ev::lift();
		return Ev::yield().then([pvalue, it]() {
			return it->callback(*pvalue);
		}).then([pvalue, it]() {
			return raise_loop(std::move(pvalue), it->next.get());
		});
	}
public:
	/* `a` must be at least movable.
	 * The general expected use-case is that `a` is a
	 * very dumb data-only structure.
	 */
	Ev::Io<void> raise(a value) {
		/* Move it to something we can share easily.  */
		auto pvalue = std::make_shared<a>(std::move(value));

		return raise_loop(std::move(pvalue), first.get());
	}

	void subscribe(std::function<Ev::Io<void>(a const&)> callback) {
		if (!callback)
			return;
		auto nnode = Util::make_unique<Node>();
		nnode->callback = std::move(callback);
		if (last) {
			last->next = std::move(nnode);
			last = last->next.get();
		} else {
			first = std::move(nnode);
			last = first.get();
		}
	}
};

}}

#endif /* !defined(S_DETAIL_SIGNAL_HPP) */
