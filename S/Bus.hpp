#ifndef S_BUS_HPP
#define S_BUS_HPP

#include<S/Detail/Signal.hpp>
#include<typeindex>
#include<typeinfo>

namespace S {

/** class Bus
 *
 * @brief signal bus for broadcasting messages and
 * subscribing to broadcasts.
 */
class Bus {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Bus();
	Bus(Bus&&);
	~Bus();

private:
	/* Maps message type to Signal object.
	 * If the given type has no Signal object yet,
	 * call the make function.
	 */
	S::Detail::SignalBase&
	get_signal( std::type_index type
		  , std::function< std::unique_ptr<S::Detail::SignalBase>()
				 > make
		  );
	/* Type-safe lookup of Signal object.  */
	template<typename a>
	S::Detail::Signal<a>& get_signal_ex() {
		using Signal = S::Detail::Signal<a>;
		static auto type = std::type_index(typeid(Signal));
		static auto make = []() {
			return Util::make_unique<Signal>();
		};
		auto& sbase = get_signal(type, make);
		return static_cast<Signal&>(sbase);
	}
public:
	template<typename a>
	void subscribe(std::function<Ev::Io<void>(a const&)> cb) {
		get_signal_ex<a>().subscribe(std::move(cb));
	}
	template<typename a>
	Ev::Io<void> raise(a value) {
		return get_signal_ex<a>().raise(std::move(value));
	}
};

}

#endif /* !defined(S_BUS_HPP) */
