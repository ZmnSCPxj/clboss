#ifndef EV_MEMOIZE_HPP
#define EV_MEMOIZE_HPP

#include"Ev/Io.hpp"
#include<functional>
#include<map>
#include<tuple>
#include<utility>

/* This provides an Ev::memoize(f) function, where
 * f is a functor type that accepts any number of
 * arguments and returns an Ev::Io<r>.
 *
 * This returns a functor which accepts the same
 * number of arguments, and returns an Ev::Io<r>,
 * but ensures that the input functor is only
 * invoked once for each distinct set of arguments.
 *
 * The arguments must be less-than-comparable and
 * copyable, and the return type a must be copyable.
 */

namespace Ev {

namespace Detail {

/* Base class for memoization.  */
template< typename r
	, typename... as
	>
class MemoizerBase {
public:
	typedef std::function<Ev::Io<r>(as...)> FuncType;

private:
	typedef std::tuple<as...> ArgsTuple;
	typedef std::map<ArgsTuple, r> TableType;
	typedef std::shared_ptr<TableType> PImpl;
	PImpl pimpl;
	std::shared_ptr<FuncType> func;

protected:
	explicit
	MemoizerBase( FuncType func_
		    ) : pimpl(std::make_shared<TableType>())
       		      , func(std::make_shared<FuncType>(std::move(func_)))
		      { }

public:

	Ev::Io<r> operator()(as... args) const {
		auto my_pimpl = pimpl;
		auto my_func = func;
		return Ev::lift().then([my_pimpl, my_func, args...]() {
			auto key = std::make_tuple(args...);
			auto it = my_pimpl->find(key);
			if (it != my_pimpl->end())
				return Ev::lift(it->second);

			return (*my_func)(args...).then([my_pimpl, key](r result) {
				my_pimpl->emplace(key, result);
				return Ev::lift(std::move(result));
			});
		});
	}
};

/* Intermediate class for memoizing functor classes.  */
template<typename f, typename op>
class MemoizerFunctor;

template<typename f, typename r, typename... as>
class MemoizerFunctor<f, Ev::Io<r>(f::*)(as...) const> : public MemoizerBase<r, as...> {
public:
	MemoizerFunctor( f func_
		       ) : MemoizerBase<r, as...>(std::move(func_)) { }
};

/* Concrete class for memoization.  */
template<typename f>
class Memoizer : public MemoizerFunctor<f, decltype(&f::operator())> {
public:
	Memoizer( f func_
		) : MemoizerFunctor< f
				   , decltype(&f::operator())
				   >(std::move(func_)) { }
};

}

template<typename f>
typename Detail::Memoizer<f>::FuncType
memoize(f func_) {
	return Detail::Memoizer<f>(std::move(func_));
}

}

#endif /* !defined(EV_MEMOIZE_HPP) */
