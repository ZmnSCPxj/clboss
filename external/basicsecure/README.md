This tiny library contains a small amount of code to implement a few
important secure operations:

* Constant-time compare.
* Non-optimizable memory zeroing.
* Secure random number acquisition.

NOTE: Because of the use of `/dev/urandom`, on old Linux systems that
predate the `getrandom` syscall, this library is not safe to use in
early boot before your OS has read the random number seed or entropy
file.
On most modern `init` systems, if you need to start up a daemon that
uses this library, you can probably make it dependent on the service
that reloads the random number seed or entropy file.

Finally, on modern Linuxen this problem goes away since the `getrandom`
syscall doees The Right Thing and waits for the random number generator
to start before returning, but does not block afterwards.
`getrandom` has been present since 3.16, for reference as of this time
the latest Linux kernel is 5.11.
On the other hand, the detection of the syscall might not be perfect.

On most BSDs this problem never existed since `/dev/urandom` is used
and on BSD the `/dev/urandom` interface matches the Linux `getrandom`
behavior --- it blocks at startup until entropy is loaded, but once
the initial seed entropy is loaded the interface no longer blocks.

On the other hand, in some containerized environments `/dev/urandom`
may not be accessible and this library will simply abort the program
in that case.
