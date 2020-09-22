#ifndef S_DETAIL_SIGNAL_HPP
#define S_DETAIL_SIGNAL_HPP

#include<Ev/Io.hpp>
#include<Ev/concurrent.hpp>
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
		std::shared_ptr<Node> next;
	};
	std::shared_ptr<Node> first;
	Node* last;

public:
	Signal() : first(), last(nullptr) { }

private:
	struct RaiseData {
		std::unique_ptr<a> value;
		std::function<void()> pass;
		std::function<void(std::exception_ptr)> fail;
		std::exception_ptr exc;
		bool starting;
		std::size_t running;
		explicit
		RaiseData(a value_
			 ) : value(Util::make_unique<a>(std::move(value_)))
			   , pass(nullptr)
			   , fail(nullptr)
			   , exc(nullptr)
			   , starting(true)
			   , running(0)
			   { }
		void finish_startup() {
			starting = false;
			if (running == 0)
				trigger();
		}
		void finish_raise() {
			--running;
			if (!starting && running == 0)
				trigger();
		}
		void trigger() {
			value = nullptr;
			if (exc) {
				pass = nullptr;
				fail(exc);
			} else {
				fail = nullptr;
				pass();
			}
		}
	};

	static
	Ev::Io<void> raise_loop( std::shared_ptr<RaiseData> pdata
			       , std::shared_ptr<Node> it
			       ) {
		if (!it)
			return Ev::lift().then([pdata]() {
				return Ev::Io<void>([ pdata
						    ]( std::function<void()> pass
						     , std::function<void(std::exception_ptr)> fail
						     ) {
					pdata->pass = std::move(pass);
					pdata->fail = std::move(fail);
					pdata->finish_startup();
				});
			}).then([]() {
				return Ev::yield();
			});

		return Ev::yield().then([pdata, it]() {
			++pdata->running;
			auto action = Ev::Io<void>([ pdata, it
						   ]( std::function<void()> pass
						    , std::function<void(std::exception_ptr)> _
						    ) {
				it->callback(*pdata->value).run([ pdata
								, pass
								]() {
					pass();
					pdata->finish_raise();
				}, [pdata, pass](std::exception_ptr e) {
					pass();
					pdata->exc = e;
					pdata->finish_raise();
				});
			});
			return Ev::concurrent(action);
		}).then([pdata, it]() {
			return raise_loop(pdata, it->next);
		});
	}
public:
	/* `a` must be at least movable.
	 * The general expected use-case is that `a` is a
	 * very dumb data-only structure.
	 */
	Ev::Io<void> raise(a value) {
		/* Move it to something we can share easily.  */
		auto pdata = std::make_shared<RaiseData>(std::move(value));

		return raise_loop(std::move(pdata), first);
	}

	void subscribe(std::function<Ev::Io<void>(a const&)> callback) {
		if (!callback)
			return;
		auto nnode = std::make_shared<Node>();
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
