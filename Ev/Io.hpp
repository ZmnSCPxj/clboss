#ifndef EV_IO_HPP
#define EV_IO_HPP

#include<exception>
#include<functional>
#include<memory>
#include<type_traits>

namespace Ev {

/* Pre-declare for Detail::IoInner.  */
template<typename a>
class Io;

namespace Detail {

/* Given an Io<a>, extract the type a.  */
template<typename t>
struct IoInner;
template<typename a>
struct IoInner<class Io<a>> {
	using type = a;
};
/* Given a type a, give the std::function that accepts that type.  */
template<typename a>
struct PassFunc {
	using type = std::function<void(a)>;
};
template<>
struct PassFunc<void> {
	using type = std::function<void()>;
};

/* Base for Io<a>.  */
template<typename a>
class IoBase {
public:
	typedef
	std::function<void ( typename Detail::PassFunc<a>::type
			   , std::function<void (std::exception_ptr)>
			   )> CoreFunc;

protected:
	CoreFunc core;

	template <typename b>
	friend class Ev::Io;
	template <typename b>
	friend class IoBase;

public:
	IoBase(CoreFunc core_) : core(std::move(core_)) { }

	template<typename e>
	Io<a> catching(std::function<Io<a>(e const&)> handler) const {
		auto core_copy = core;
		return Io<a>([ core_copy
			     , handler
			     ]( typename Detail::PassFunc<a>::type pass
			      , std::function<void (std::exception_ptr)> fail
			      ) {
			auto sub_fail = [ pass, fail
					, handler
					](std::exception_ptr err) {
				try {
					std::rethrow_exception(err);
				} catch (e const& err) {
					handler(err).core(pass, fail);
				} catch (...) {
					fail(std::current_exception());
				}
			};
			core_copy(pass, sub_fail);
		});
	}
};

}

template<typename a>
class Io : public Detail::IoBase<a> {
public:
	Io(typename Detail::IoBase<a>::CoreFunc core_) : Detail::IoBase<a>(std::move(core_)) { }

	/* (>>=) :: IO a -> (a -> IO b) -> IO b*/
	template<typename f>
	Io<typename Detail::IoInner<typename std::result_of<f(a)>::type>::type>
	then(f func) const {
		using b = typename Detail::IoInner<typename std::result_of<f(a)>::type>::type;
		auto core_copy = this->core;
		/* Continuation Monad.  */
		return Io<b>([ core_copy
			     , func
			     ]( typename Detail::PassFunc<b>::type pass
			      , std::function<void (std::exception_ptr)> fail
			      ) {
			try {
				auto sub_pass = [func, pass, fail](a value) {
					func(std::move(value)).core(pass, fail);
				};
				core_copy(sub_pass, fail);
			} catch (...) {
				fail(std::current_exception());
			}
		});
	}

	void run( typename Detail::PassFunc<a>::type pass
		, std::function<void (std::exception_ptr)> fail
		) const noexcept {
		auto completed = std::make_shared<bool>(false);
		auto sub_pass = [completed, pass](a value) {
			if (!*completed) {
				*completed = true;
				pass(std::move(value));
			}
		};
		auto sub_fail = [completed, fail](std::exception_ptr e) {
			if (!*completed) {
				*completed = true;
				fail(std::move(e));
			}
		};
		try {
			this->core(std::move(sub_pass), std::move(sub_fail));
		} catch (...) {
			if (!*completed) {
				*completed = true;
				fail(std::current_exception());
			}
		}
	}
};

/* Create a separate then-implementation for Io<void> */
template<>
class Io<void> : public Detail::IoBase<void> {
public:
	Io(typename Detail::IoBase<void>::CoreFunc core_) : Detail::IoBase<void>(std::move(core_)) { }

	/* (>>=) :: IO () -> (() -> IO b) -> IO b*/
	template<typename f>
	Io<typename Detail::IoInner<typename std::result_of<f()>::type>::type>
	then(f func) const {
		using b = typename Detail::IoInner<typename std::result_of<f()>::type>::type;
		auto core_copy = core;
		/* Continuation Monad.  */
		return Io<b>([ core_copy
			     , func
			     ]( typename Detail::PassFunc<b>::type pass
			      , std::function<void (std::exception_ptr)> fail
			      ) {
			try {
				auto sub_pass = [func, pass, fail]() {
					func().core(pass, fail);
				};
				core_copy(sub_pass, fail);
			} catch (...) {
				fail(std::current_exception());
			}
		});
	}

	void run( typename Detail::PassFunc<void>::type pass
		, std::function<void (std::exception_ptr)> fail
		) const noexcept {
		auto completed = std::make_shared<bool>(false);
		auto sub_pass = [completed, pass]() {
			if (!*completed) {
				*completed = true;
				pass();
			}
		};
		auto sub_fail = [completed, fail](std::exception_ptr e) {
			if (!*completed) {
				*completed = true;
				fail(std::move(e));
			}
		};
		try {
			this->core(std::move(sub_pass), std::move(sub_fail));
		} catch (...) {
			if (!*completed) {
				*completed = true;
				fail(std::current_exception());
			}
		}
	}
};

template<typename a>
Io<a> lift(a val) {
	auto container = std::make_shared<a>(std::move(val));
	return Io<a>([container]( std::function<void(a)> pass
				, std::function<void(std::exception_ptr)> fail
				) {
		pass(std::move(*container));
	});
}
inline
Io<void> lift(void) {
	return Io<void>([]( std::function<void()> pass
			  , std::function<void(std::exception_ptr)> fail
			  ) {
		pass();
	});
}

}

#endif /* !defined(EV_IO_HPP) */
