`Ev::Io`
========

`Ev::Io` is a framework for greenthreads.

A greenthread is like a thread, except it is handled by the application
and not the operating system, and thus all greenthreads will share the
same single CPU resource.
That is, as far as the OS is concerned the application is single-
threaded.

The interface of `Ev::Io` is similar to the Javascript `Promise` type.
An `Ev::Io<int>` is, like a Javascript `Promise`, not an actual `int`,
but instead some incomplete computation that will eventually yield an
`int`.
You can then extend the pending `Ev::Io` by using the `then` member
function, passing in a function of your own, which will (eventually)
get called with the result.
The function you pass in to `then` should then return another `Ev::Io`
(whose type parameter can be different), and the result of `then` will
have the same type.

```C++
Ev::Io<std::string> stringify(Ev::Io<int> io_int) {
	return std::move(io_int).then([](int x) {
		auto os = std::ostringstream();
		os << x;
		auto str = os.str();
		return Ev::lift(std::move(str));
	});
}
```

The function `Ev::lift` is similar to the Javascript `Promise.resolve`
and "lifts" a non-`Ev::Io` value into one that is trivially wrapped
in an `Ev::Io`, i.e.

    template<typename a>
    Ev::Io<a> Ev::lift(a);

The name `Io` is from Haskell `IO` type.
Javascript `Promise`s are ultimately monads, and Haskell `IO` is
one of the first monads labelled as such ever encountered by most
programmers.
`Ev::lift` is equivalent to Haskell `return`.
And the `Ev::Io<a>::then` member function is equivalent to Haskell
`>>=`.

Generally, when using `Ev::Io`, the expectation is that most of
your computation is performed from "within" the `Ev::Io` framework.

The namespace name `Ev` is from the use of `libev`, although in
principle any event-based framework can be used.

Using
-----

An `Ev::Io<a>` for any type `a` can be understood primarily as
"an action that will return the type `a`".

There is also an `Ev::Io<void>` specialization.
This is for an action that is performed within the `Ev::Io`
framework, but which does not in fact need a return value.

The `Ev::lift` function has an overload without any arguments,
which yields an `Ev::Io<void>`.

It is also possible to `+` any number of `Ev::Io<void>` together
to create another `Ev::Io<void>`.
This results in an `Ev::Io<void>` action that is "the left-hand
side action executed to completion, followed by the right-hand
side action executed to completion".

As mentioned above, an action `Ev::Io<a>` for all types `a` has a
member function `then` which constructs a new action.
Thie `then` function is passed a function, which should accept the
type `a` and returns `Ev::Io<b>` for any type `b`.

An action `Ev::Io<a>` for all types `a` also has a member
function `catching`.
This is a templated function which is parametrized with a
specific exception type.
The `catching` function is passed a function, which should accept
by `const&` the exception type and return the same type
`Ev::Io<a>`.
If the action would throw the parametrized exception type, then
the resulting action from `catching` is the same action, but with
the exception passed to the given function and the result of that
function as the action performed.

If a function passed into `then` or `catching` would `throw` an
exception before returning its `Ev::Io` result, then the resulting
action would throw an exception "inside" the `Ev::Io` framework,
which can be caught using a `catching` function call.

Return Value Optimization
-------------------------

In general, you should *avoid* something like this in C++:

```C++
std::string cat(std::string a, std::string b) {
	auto res = std::move(a) + std::move(b);
	return std::move(res); /* DISABLES RVO!!!  Not do this!!  */
}
```

RVO gets disabled in the above code since the returned value
is from a temporary.
RVO only triggers if all returns in a function in all paths
will return the same local variable in the function.
The wrapping `std::move` prevents this optimization.

However, within `Ev::Io`, you *should* use `std::move` to
return a local variable.

```C++
Ev::Io<std::string> cat(std::string a, std::string b) {
	auto res = std::move(a) + std::move(b);
	return Ev::lift(std::move(res));
}
```

RVO cannot work here anyway since you always have to use `Ev::lift`
to return a wrapping `Ev::Io` around the value.
At least with `std::move` you can ensure that the same underlying
memory area for a container can be simply transferred from the local
variable to the temporary object that `Ev::lift` creates to hold the
return value.

Running Inside `Ev::Io`
-----------------------

It is actually better practice to always use `Ev::lift()` in order
to defer operations until the action is actually performed:

```C++
Ev::Io<std::string> cat(std::string a, std::string b) {
	return Ev::lift().then([a, b]() {
		auto res = a + b;
		return Ev::lift(std::move(res));
	});
}
```

This is significant if the code being run modifies any mutable
variables, such as those on the object that the function is being
called on.
Without the above wrapping, the updates to the mutable variables
are performed immediately, whereas with the above wrapping, any
writes to the mutable variables will be deferred to when the
action is actually executed.
This is significant if e.g. you intend the action to be performed
in a different greenthread.

As good practice you should probably always wrap around with the
simple `Ev::lift().then([vars ...]() { ... })`.

In the above example, the input strings `a` and `b` may be very
large, and the copying may be costly.
You could use `std::shared_ptr` to move objects and share the
same object to the inside of the wrapper:

```C++
Ev::Io<std::string> cat(std::string a_, std::string b_) {
	auto a = std::make_shared<std::string>(std::move(a_));
	auto b = std::make_shared<std::string>(std::move(b_));

	return Ev::lift().then([a, b]() {
		auto res = std::move(*a) + std::move(*b);
		return Ev::lift(std::move(res));
	});
}
```

Tail Calls
----------

Tail calls of `Ev::Io` get optimized even if the compiler itself
does not perform tail call optimization.
The only caveat here is that you need to call into `Ev::yield` at
some point in a tail-recursive loop, otherwise the C stack will
fill with stack frames; the `Ev::yield` resets the C stack.
See `tests/ev/test_io_mem_leak.cpp`.

For example, if you are going to use the previous recommendation
to always wrap your code inside `Ev::lift().then(...)`,
you can "just" replace the `Ev::lift()` with `Ev::yield()` for
functions that require tail calls.

This is magically done by the operation of `Ev::Io`; code that
would receive the result of an `Ev::Io` action do not require any
C stack space to store variables, instead they are objects
allocated in heap space, and which the `Ev::Io` framework controls.
The framework provides tail-call optimization "for free".

If you need to loop around an operation that returns an `Ev::Io`
action, then that is most naturally done using tail-recursion.
Just make sure to use `Ev::yield`, most naturally at the start of
any recursive function.

Single-Threaded Concurrency
---------------------------

The greenthreads provided by `Ev::Io` allows for concurrency
without launching multiple OS-level threads.
Greenthreads are fairly cheap (require no syscall, has fewer "gotcha"
semantics for file descriptors and `fork` and  `exec`, etc.).

You can use the `Ev::concurrent` function to launch a new
greenthread.
This returns an action which creates the new greenthread and
schedules it to be launched when the main thread is idle and has
no other actions to do.

Actions you pass into `Ev::concurrent` should really be wrapped
in an `Ev::lift().then(...)`, to ensure that the code only gets
executed when the greenthread actually starts.

Now, obviously when you have multiple concurrent greenthreads of
execution, then you need some way to somehow "lock" resources.

A thing to note is that within a single `then` function, other
concurrent greenthreads are not executing.
This means that all operations within a single function that you
are passing into a `then` call are atomic.
You can then trivially use this to implement locks.

The below is a lousy spinlock (do not use it, it causes 100%
CPU due to spinlocking):

```C++
class Lock {
private:
	bool flag;

public:
	Lock() : flag(false) { }

	Ev::Io<void> acquire() {
		return Ev::lift().then([this]() {
			/* Flag is set, spin.  */
			if (flag)
				/* Let other greenthreads run,
				 * then try again.
				 */
				return Ev::yield()
				     + acquire()
				     ;
			flag = true;
			return Ev::lift();
		});
	}
	Ev::Io<void> release() {
		return Ev::lift().then([this]() {
			flag = false;
			return Ev::lift();
		});
	}
};
```

No additional locking is needed, as within a `then`, all code
is atomic relative to other greenthreads.

In fact, strictly speaking, you can (probably) list down the
operations that could allow other greenthreads to execute.
`Ev::yield()`, for example, definitely allows other greenthreads
to execute.
Some blocking operations are likely to be implemented in such a
way as to allow other greeenthreads to be run as well.
If you do not use such operations, then your code will still
run atomically.

For example, since `Ev::lift` does not let other greenthreads
run:

```C++
	Ev::Io<void> acquire() {
		return Ev::lift().then([this]() {
			return Ev::lift(flag);
		}).then([this](bool flag_read) {
			/* Flag is set, spin.  */
			if (flag_read)
				/* Let other greenthreads run,
				 * then try again.
				 */
				return Ev::yield()
				     + acquire()
				     ;
			/* No TOCTOU race here!  */
			flag = true;
			return Ev::lift();
		});
	}
```

Still, as a rule, if you ensure that you never end a particular
`then`, you can assume that no other greenthreads will be running.
If you have to end a `then`, then it is safest to assume that
other greenthreads may or may not run during the break.

### `Ev::Semaphore`

We also have an existing semaphore implementation, which you can
use for locks.
A semaphore allows up to N greenthreads to run simultaneously
within the semaphore, with the number N selected at construction
of the semaphore.
The semaphore can be used as a mutex by configuring it with N=1.

The interface does not have explicit "increment" or "decrement"
interfaces like typical semaphores.
Instead, you hand over any action `Ev::Io<a>` where `a` is any
type to its `run` member function, and get back an action of
the same type.
The semaphore will then ensure that only up to `N` such actions
will run simultaneously even across `Ev::concurrent`.

The semaphore is designed so that any extra greenthreads that
are blocked on the semaphore will not consume CPU.

`Ev::ThreadPool`
----------------

Some POSIX interfaces are really atrocious.

For example, `gethostbyname` blocks the calling thread on a
network interface, without exposing a file descriptor you can
wait on or anything useful like that so you can handle other
network things in the meantime.
You should avoid similar interfaces in `Ev::Io` code, because
it will block *all* greenthreads.

However, sometimes you do need to actually use such interfaces.
Typically, most frameworks that need to wrap such atrocious
blocking interfaces work by executing in a background thread.
`Ev::Io` provides the `Ev::ThreadPool` object, a generalization
of this technique.

Construct a single `Ev::ThreadPool` for your entire program,
and pass in a reference to it to all parts of your program
that need to run lengthy blocking functions.
Then you can just use the `background` member function to run a
function in the background.

```C++
Ev::Io<struct hostent*> io_gethostbyname( Ev::ThreadPool& tp
					, std::string const& name
					) {
	return tp.background<struct hostent*>([name]() {
		return gethostbyname(name);
	});
}
```

The above returns a "normal" `Ev::Io` action.
As long as the core `gethostbyname` is blocked in one of the
background threads, the main OS thread will keep running and
other greenthreads have an opportunity to execute on the main
OS thread.
If no other greenthreads are pending, no CPU will be consumed
by the main OS thread, only the background thread running the
task.

Notice that the function you pass to `background` returns a
"plain" C++ object, not one wrapped in `Ev::Io<a>`.

`Ev::ThreadPool` is not intended for compute-heavy tasks; in
particular, it launches a fixed number of threads without
checking how many processors are actually available, or
considering the CPU load of the entire system.
It is really intended only to make blocking interfaces safely
non-blocking on the main OS thread to allow the greenthread
system to work correctly.

Starting
--------

It is easy to move from normal C++-space to `Ev::Io`-space.
`Ev::lift` promotes anything from plain movable C++ object to an
`Ev::Io` action.
Similarly, `throw`n exceptions from within a `then` or `catching`
are also automatically lifted to exceptions within `Ev::Io`.

So if we have an action that represents our entire program, how do
we actually **run** the action in order to run our entire program?

This is done by the `Ev::start` function.
This accepts an `Ev::Io<int>` and returns an `int`.

This is how it is intended to be used:

```C++
#include"Ev/Io.hpp"
#include"Ev/start.hpp"

Ev::Io<int> real_main(int argc, char** argv);

int main(int argc, char** argv) {
	return Ev::start(real_main(argc, argv));
}
```

`real_main` should be the entire program as a single action.

An alternate way is to call the `run` member function on `Ev::Io`,
but this is discouraged.
`Ev::start` properly initializes `libev`, which is needed in order
for `Ev::concurrent`, `Ev::yield`, and others to work properly.
This member function really only exists for testing, and for
implementing `Ev::start` itself.

Implementation
--------------

Internally, an `Ev::Io<a>` is really just a function that accepts
two functions, a `pass` continuation and a `fail` continuation.
That is, `Ev::Io` is just continuation-passing style.

An `Ev::Io<a>` is really just:

```C++
std::function< void( std::function<void(a)> // pass
		   , std::function<void(std::exception_ptr)> // fail
		   )
	     > func;
```

Indeed, the formal public constructor for `Ev::Io<a>` accepts the
above type as the sole argument (note that Javascript `Promise` is
the same as well).
The `then` and `catching` member functions just wrap the function
further.

This works well with any event-based system (meaning we could use
something other thhan `libev` as a backend).
To implement a blocking operation, you instead construct an
`Ev::Io<a>` that is given a function of the above type.
That function should save the `pass` and `fail` functions in a
heap-allocated object, then register that object into the event
system to wait for the event.

Then, when the event triggers, the object recovers the `pass`
and `fail` functions, release any resource held by the object,
then calls the appropriate continuation based on the event.
See `Ev/yield.cpp` For example: this simple example registers an
"idle" event for when the main loop is idle, then resumes
computation from there.

This is the source of some of the magic.

For example, tail call optimization is very natural with a
continuation-passing style.
When an `Ev::Io`-returning function calls itself, the call
actually just creates an action (that performs the code
wrapped inside the `Ev::Io`) and returns immediately without
recursing.

Inside the action, when the action calls the same function
again, the resulting action is passed the same `pass` and
`fail` continuations, without requiring any additional
memory allocations to create wrappers for those continuations.
This implies that no additional memory is needed for tail
calls, which is all that is needed for tail call optimization.
