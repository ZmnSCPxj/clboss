Boss Modules
============

This directory contains modules of CLBOSS.

CLBOSS (`Boss::Main`) is constructed as a bunch of modules centered
around a common bus, the `S::Bus`.

Each module is given access to the `S::Bus`, and can `subscribe` to
any messages on the bus.
It can also `raise` any messages on the bus.

A module is just a generic object.
The only requirement is that it be constructible and destructible.

Ideally, modules should not have references or pointers to other
modules; instead, it interacts with other modules by `raise`-ing
messages, and responding to `subscribe`d messages.
There are a few modules which violate this rule, mostly ones from
the early days of CLBOSS when I was much less confident of this
design, but those few --- mostly `Boss::Mod::Rpc` and
`Boss::Mod::Waiter` --- should eventually be migrated to using
`S::Bus` fully.

Module Header
-------------

Below is a sketch of what a new module would have in its header:

```C++
#ifndef BOSS_MOD_NEWMODULE_HPP
#define BOSS_MOD_NEWMODULE_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::NewModule
 *
 * @brief this is a simple example of a new module.
 */
class NewModule {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	/* We need to be constructed explicitly with a
	 * S::Bus!  */
	NewModule() =delete;

	/* These will be defined in the source file using
	 * `default`.
	 */
	NewModule(NewModule&&);
	~NewModule()

	/* The constructor.  */
	explicit
	NewModule(S::Bus&);
};

}}

#endif /* !defined(BOSS_MOD_NEWMODULE_HPP) */
```

Yes, it is really that simple.
There is no need to define anything in the header other than the
fact that the module exists and can be moved and destructed and
can be constructed with a `S::Bus`.

The meat of the module will be defined in the source file, in the
`Boss::Mod::NewModule::Impl` implementation class.

(The few modules which do not use this pattern were from earlier
days before I realized this pattern worked best.)

Module Source
-------------

A minimal new module has this code in the corresponding source
file:

```C++
#include"Boss/Mod/NewModule.hpp"
#include"Ev/Io.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"

namespace Boss { namespace Mod {

/* This contains all the data and code that the new module
 * actually needs.
 */
class NewModule::Impl {
private:
	S::Bus& bus;

	int your_data_here;

	void start() {
		your_data_here = default_your_data_value;
		bus.subacribe<Boss::Msg::Whateverness
			     >([this](Boss::Msg::Whateverness const& msg) {
			return do_whatever(msg);
		});
		// ...
	}

	Ev::Io<void> do_whatever(Boss::Msg::Whateverness const& msg) {
		// ...
		return Ev::lift();
	}

public:
	/* The below are not really necessary (this source file is
	 * the only one that will see the `Impl` class anyway) but I
	 * feel they are necessary.  */
	Impl() =delete;
	Impl(Impl&&) =delete;

	explicit
	Impl(S::Bus& bus_) : bus(bus_) { start(): }
};

/* The C++-generated defaults should work fine.  We just need to
 * let the compiler know about the `Impl` class first before the
 * compiler can generate the `default`, which is why the `Impl`
 * class definition above comes first.
 */
NewModule::NewModule(NewModule&&) =default;
NewModule::~NewModule() =default;

/* Just construct the implementation.  */
NewModule::NewModule(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
```

So, as mentioned, the `Impl` class contains all that your module
***is***.
It declares the data and the code of your module.

From within your module, you can `subscribe` to and `raise` messages.
Those are the ways by which your module can affect the rest of the
modules.

Module Registration
-------------------

Module code is nice, but they do need to be instantiated in the
initial object graph before the module can start interacting with the
rest of CLBOSS.

In order to do that, you need to add it to `Boss/Mod/all.cpp`.

Just `#include` the header file, then add an `install` decleration:

    all->install<NewModule>(bus);

The order in which modules are installed into the instantiated CLBOSS
should not matter.
However, some old modules may be sensitive to order (due to being
designed with dependencies, but "modern" CLBOSS should have modules
depend only on a common interface, the bus, i.e. Dependency Inversion),
so avoid reordering existing modules (but feel free to insert new
modules wherever, do note that the modules are roughly grouped so try
to locate the most logical group for the module and add it there).

Shutting Down
-------------

When the `lightningd` shuts down, it closes the stdin of `clboss`.

This causes `Boss::JsonInput::run` (the main loop) to return, which
then causes the special message `Boss::Shutdown` to be emitted on
the bus.

Some blocking interfaces are connected to objects that also monitor
this message on the `S::Bus`.
Below is not an exhaustive list but probably has all the important
ones you would use.

* `Boss::Mod::Rpc::command`
* `Boss::Mod::Waiter::wait`
* `Boss::ModG::ReqResp<MsgType>::execute`

When the `Boss::Shutdown` message is broadcasted on the bus, the
above interfaces will throw an `Ev::Io`-level exception, of the
`Boss::Shutdown` type (yes, the same as the message --- it is just
some type, and that is all C++ cares about).

The proper thing to do is to simply not handle `Boss::Shutdown`.
This causes the `Ev::Io`-level greenthread your module code is
running in, to terminate with an uncaught exception, which allows
CLBOSS to exit cleanly once all greenthreads have terminated.

There exists a `Boss::concurrent` function that you should use for
launching `Ev::Io`-level greenthreads.
This simply catches any uncaught `Boss::Shutdown` in the spawned
greenthread and exits the greenthread when this is caught.
Otherwise, the user will get a `uncaught exception` scare-message
on stderr, which is benign (since the exception only exists to
unblock blocking interfaces).

`S::Bus::raise`, `Ev::map`,  `Ev::foreach`, which do not use
`Boss::concurrent` to launch new `Ev::Io`-level greenthreads,
are fine since any `Ev::Io`-level exception --- including a thrown
`Boss::Shutdown` from the above blocking interfaces --- from any
sub-greenthread they launch will propagate it to the calling
thread as well, which should eventually reach out to the
`Boss::concurrent` that launched whatever greenthread your module
started in.

Thus, for the most part, shutting down "just works" and you can
ignore it, usually.

However, if you have code that has to wait for something (and
that something is not one of the above blocking interfaces),
then take note that you should have a long-lived object that
monitors for `Boss::Shutdown` and forces your code to stop
waiting and throws `Boss::Shutdown` to the caller.
For example, it could set a flag somewhere in your object, and
your waiting code should also ccheck this flag (meaning your
waiting code has to poll this flag as well, and throw
`Boss::Shutdown` if the flag is set).

Lifetime Management
-------------------

Managing the lifetimes of objects is fairly important in C++.

Modules registered in the `Boss::Mod::all` function are
*persistent*; they will be alive until after `Boss::Shutdown`
is broadcast on the bus and the above blocking interfaces have
thrown the exception at the `Ev::Io` level.

Of course, if you use objects in your own code, you do need to
be responsible for managing their lifetimes, usually by use of
standard `std::unique_ptr` and `std::shared_ptr`.

Typically it means you need to have a container in your module
that contains `std::unique_ptr`s to objects that have less than
*persistent* lifetime.
Then when you know that the object lifetime is over, you can
remove the object from the container.

Sometimes, you might want to launch a background `Ev::Io`
greenthread with its own object, and have the object
automatically be reaped when the greenthread exits.
For that case, you can take advantage of C++ lambda captures:

```C++
Ev::Io<void> do_something_in_foreground() {
	auto temp_object = std::make_shared<TempObject>();
	return Ev::lift().then([temp_object]() {
		return temp_object->do_something();
	}).then([temp_object]() {
		/* This function only exists to keep the
		 * `temp_object` alive until the above
		 * function completes.
		 */
		return Ev::lift();
	});
}
Ev::Io<void> do_something() {
	return Boss::concurrent(do_something_in_foreground());
}
```

The above `do_something` function will then launch the task in
some background greenthread.
The `TempObject` remains alive until its `do_something` member
function returns.
The nice thing is that even if the `TempObject::do_something`
throws an uncaught exception (like the aforementioned
`Boss::Shutdown`), the `temp_object` will also be destructed
correctly.

When using the above pattern, be careful with all uses of
`Boss::concurrent` from within the *temporary* objects.
If the `Boss::concurrent` outlasts the temporary object, then
it might refer to objects that the temporary object is
keeping alive, and which might be dead at that point.
See commit `f01a730e` for an example of this bug.

An important thing to note is that **only *persistent* objects
can `subscribe` to the `S::Bus`**.
Temporary objects ***cannot*** `subscribe` to any messages.
If a temporary object like the above `TempObject` needs to
listen for messages, your module (being *persistent*) should
listen on behalf of those objects, then either forward it to
any `TempObject`, or allow `TempObject` to somehow access the
contents of messages.

If you have a temporary object `subscribe` on the bus, then
when the object dies and is destructed, the bus will still
forward any raised messages to its function, which would, with
high probability, attempt to access its previous memory area
and lead to use-after-free bugs.

This is a limitation of the current `S::Bus`; in principle it
should be possible to add a sort of "leased subscribe" which
would return an object that your temporary object will keep
alive, whose destruction would unsubscribe the corresponding
function.

`Boss::ModG::ReqResp`
---------------------

Often, a module keeps track of some information about the
universe, or may serve as wrappers around some functionality
that can be replaced (most likely for mocking, so that we can
write module-level tests by instantiating just the module
under test and adding code to subscribe and raise messages on
the test harness.
See the tests in `tests/boss`).

Obviously, other modules then need to somehow "call" into
the module, to query the data or to trigger the functionality.

In a typical object graph this is done by having the caller
object somehow get access to the interface of the callee
object.

However, with the `S::Bus`, the caller can instead just be
given the very generic `S::Bus` interface to trigger code on
the callee object, and get a response.

To facilitate this, the `Boss::ModG::ReqResp` templated class
exists.
Simply instantiate this in your module --- it has to be a
*persistent* object with the same lifetime as your module,
since it `subscribe`s to messages --- and call its `execute`
member function.
This function will send the `Req` message (the first argument
of the template) and will wait for the corresponding `Resp`
message, and will block the calling greenthread until the
corresponding response is received.
It will also handle `Boss::Shutdown` correctly --- any blocked
greenthread will experience `Boss::Shutdown` being thrown to
unblock the greenthread.

Now, `Boss::ModG::ReqResp` is not magic.
Any `Req` message and its corresponding `Resp` has to have a
`void* requester` member.
The callee object that handles this `Req` and raises the
`Resp` has to propagate the `void* requester` from the `Req`
to the `Resp` that contains the response to the given `Req`.

For example:

```C++
/* Request message.  */
struct RequestAddOne {
	void* requester;
	int x;
};
/* Response message.  */
struct ResponseAddOne {
	void* requester;
	int x_plus_1;
};
/* Callee module implementation.  */
class AddOneHandler::Impl {
private:
	S::Bus& bus;

	void start() {
		bus.subscribe< RequestAddOne
			     >([this](RequestAddOne const& m) {
			auto x_plus_1 = m.x + 1;
			return bus.raise(ResponseAddOne{
				/* Propagate the requester.  */
				m.requester,
				x_plus_1
			});
		});
	}
public:
	Impl() =delete;
	Impl(S::Bus& bus_) : bus(bus_) { start(); }
};

/* Caller module implementation.  */
class AddOneCaller::Impl {
private:
	S::Bus& bus;

	Boss::ModG::ReqResp< RequestAddOne
			   , ResponseAddOne
			   > add_one;

	void start() {
		bus.subscribe< Boss::Msg::Init
			     >([this](Boss::Msg::Init const& _) {
			return Ev::lift().then([this]() {
				/* Call into the AddOneHandler.  */
				return add_one.execute(RequestAddOne{
					/* ReqResp will fill in the
					 * requester field appropriately.  */
					nullptr,
					/* x.  */
					42
				});
			}).then([](ResponseAddOne rsp) {
				assert(resp.x_plus_1 == 43);
				return Ev::lift();
			});
		});
	}

public:
	Impl() =delete;
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      , add_one( bus_
		       , [](RequestAddOne& m, void* p) {
				m.requester = p;
			 }
		       , [](ResponseAddOne& m) {
				return m.requester;
			 }
		       )
	      { start(); }
};
```

In principle, the functions passed into the constructor of
`Boss::ModG::ReqResp` should really be in a traits class, this
should be modified at some point in the future.

A major advantage of this system is that it provides some
amount of cross-module introspection.
Any module can "tap into" the `Req` and/or `Resp` of another
module, and keep track of various statistics about how often
those bits of functionality get called.
For example, a module could be written that just subscribes
to `RequestMoveFunds` and `ResponseMoveFunds`, which then
keeps statistics on how often and how much fund movements can
actually succeed.
Then that module can expose this information.

Indeed, the other blocking interfaces, `Boss::Mod::Rpc` and
`Boss::Mod::Waiter`, do not use the `Boss::ModG::ReqResp`
framework only because they predate it.
At some point, those interfaces should be migrated to using
`Boss::ModG::ReqResp` for a single implementation of handling
the `Boss::Shutdown` message.

On-disk Data
------------

If your module keeps statistics or other information on various
events, you probably want to persist that data across restarts
of `lightningd`.

This is done by using an SQLITE3 database.
See the `Sqlite3/` directory for the internal API.

CLBOSS uses a single database, named `data.clboss`.
An `Sqlite3::Db` is broadcasted at initialization via the
`Boss::Msg::DbResource` message, which represents an
opened handle to this database.
Your module should `subscribe` to this message, and copy the
handle to its own variable.

Your module should then create a table in this database to hold
all the data that needs to persist.
Table names you create should be prefixed with the module name,
for example `NewModule_nodes`, to ensure that tables of other
modules do not conflict.

You should probably use `CREATE TABLE IF NOT EXISTS` in order
to create tables if and only if they do not exist yet.
Note that you cannot easily change the table schema --- if
your module has been released before, then users already
have the previous table schema and you cannot easily modify it.
Adding columns is possible with SQLITE3 via `ALTER TABLE`, but
changing columns or deleting them is not possible, so make sure
to think deeply about the table design before writing code for
it.
