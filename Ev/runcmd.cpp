#include"Ev/Io.hpp"
#include"Ev/runcmd.hpp"
#include"Net/Fd.hpp"
#include"Util/make_unique.hpp"
#include<errno.h>
#include<ev.h>
#include<fcntl.h>
#include<memory>
#include<poll.h>
#include<stdio.h>
#include<stdexcept>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

namespace {

class RunCmd {
private:
	std::string command;
	std::vector<std::string> argv;
	std::unique_ptr<char const*[]> exec_argv;

	std::function<void(std::string)> pass;
	std::function<void(std::exception_ptr)> fail;

	Net::Fd child_output;
	Net::Fd command_output;
	ev_io watcher;

	Net::Fd child_fail;
	Net::Fd parent_fail;

	Net::Fd devnull;

	std::string result;

public:
	RunCmd( std::string command_
	      , std::vector<std::string> argv_
	      , std::function<void(std::string)> pass_
	      , std::function<void(std::exception_ptr)> fail_
	      ) : command(std::move(command_))
		, argv(std::move(argv_))
		, pass(std::move(pass_))
		, fail(std::move(fail_))
		{
		exec_argv = Util::make_unique<char const*[]>(argv.size() + 2);
		exec_argv[0] = command.c_str();
		for (auto i = size_t(0); i < argv.size(); ++i)
			exec_argv[i + 1] = argv[i].c_str();
		exec_argv[argv.size() + 1] = nullptr;
	}
	RunCmd() =delete;

private:
	static
	void error(std::unique_ptr<RunCmd> self, std::string msg) {
		try {
			throw std::runtime_error(
				std::string("pipecmd: ") + msg +
				": " + strerror(errno)
			);
		} catch (...) {
			auto fail = std::move(self->fail);
			self = nullptr;
			fail(std::current_exception());
		}
	}
	static
	void error_child(std::unique_ptr<RunCmd> self) {
		auto err = errno;

		/* Send errno to parent.  */
		if (write(self->child_fail.get(), &err, sizeof(err)) < 0)
			/* Nothing */ ;

		/* Abnormal exit.  */
		self.release();
		_exit(1);
	}

	static bool ready_to_read(int fd) {
		auto pollarg = pollfd();
		pollarg.fd = fd;
		pollarg.events = POLLIN;
		pollarg.revents = 0;

		auto res = int();
		do {
			res = poll(&pollarg, 1, 0);
		} while (res < 0 && errno == EINTR);
		if (res < 0)
			/* Assume ready.... */
			return true;
		return (pollarg.revents & (POLLIN | POLLHUP | POLLERR)) != 0;
	}

	static
	void read_output(EV_P_ ev_io *watcher, int revents) {
		/* Re-acquire responsibility from ev.  */
		auto self = std::unique_ptr<RunCmd>((RunCmd*) watcher->data);
		ev_io_stop(EV_A_ watcher);

		char buf[256];
		while (ready_to_read(self->command_output.get())) {
			auto rres = ssize_t();
			do {
				rres = read( self->command_output.get()
					   , buf, sizeof(buf)
					   );
			} while (rres < 0 && errno == EINTR);
			if (rres < 0 && ( errno == EWOULDBLOCK
				       || errno == EAGAIN
					))
				break;
			if (rres < 0)
				return error(std::move(self), "read");
			if (rres == 0) {
				/* Command finished outputting.  */
				self->command_output = nullptr;
				break;
			}
			self->result += std::string(buf, buf + rres);
		}

		if (self->command_output) {
			/* Re-arm, re-release responsibility to ev.  */
			ev_io_start(EV_A_ watcher);
			watcher->data = self.release();
		} else {
			/* Command finished outputting.  */
			auto pass = std::move(self->pass);
			auto result = std::move(self->result);
			self = nullptr;
			pass(result);
		}
	}

public:
	static
	void run(std::unique_ptr<RunCmd> self) {
		int fds[2];
		/* Create the stdout pipe.  */
		auto res = pipe(fds);
		if (res < 0)
			return error(std::move(self), "pipe stdout");
		self->child_output = Net::Fd(fds[1]);
		self->command_output = Net::Fd(fds[0]);

		/* Create the fail pipe.
		 * This pipe is used by the child to report all failures.
		 */
		res = pipe(fds);
		if (res < 0)
			return error(std::move(self), "pipe fail");
		self->child_fail = Net::Fd(fds[1]);
		self->parent_fail = Net::Fd(fds[0]);
		/* The child end of the fail pipe should be made CLOEXEC,
		 * so that if exec succeeds it automatically closes.  */
		{
			auto flags = fcntl(self->child_fail.get(), F_GETFD);
			res = fcntl( self->child_fail.get(), F_SETFD
				   , flags | FD_CLOEXEC
				   );
		}
		if (res < 0)
			return error(std::move(self), "fcntl cloexec");

		/* Open /dev/null to serve as input to the command.  */
		auto devnull_fd = open("/dev/null", O_RDONLY);
		if (devnull_fd < 0)
			return error(std::move(self), "open /dev/null");
		self->devnull = Net::Fd(devnull_fd);

		auto pid = fork();
		if (pid < 0)
			return error(std::move(self), "fork");

		if (pid == 0) {
			/* Child.  */

			/* Close parent-side pipes.  */
			self->command_output = nullptr;
			self->parent_fail = nullptr;

			auto res = dup2( self->child_output.get()
				       , STDOUT_FILENO
				       );
			if (res < 0)
				return error_child(std::move(self));
			self->child_output = nullptr;

			res = dup2(self->devnull.get(), STDIN_FILENO);
			if (res < 0)
				return error_child(std::move(self));
			self->devnull = nullptr;

			/* Close higher fds, except the child-fail pipe.  */
			auto max = sysconf(_SC_OPEN_MAX);
			if (max > 500)
				max = 500;
			for (auto fd = int(3); fd < max; ++fd)
				if (fd != self->child_fail.get())
					close(fd);

			execvp( self->command.c_str()
			      , const_cast<char*const*>(self->exec_argv.get())
			      );
			/* Error occurred in exec.  */
			return error_child(std::move(self));
		}

		/* Close child-side pipes.  */
		self->child_output = nullptr;
		self->child_fail = nullptr;
		self->devnull = nullptr;

		/* Read any errors from the child.  */
		auto err = int();
		auto rres = read(self->parent_fail.get(), &err, sizeof(err));
		if (rres == sizeof(err)) {
			errno = err;
			return error(std::move(self), "child");
		}
		self->parent_fail = nullptr;

		/* Now we can read from command_output!  */
		ev_io_init( &self->watcher, &read_output
			  , self->command_output.get(), EV_READ
			  );
		/* Hand over responsibility to ev. */
		self->watcher.data = self.get();
		/* Start it.  */
		ev_io_start(EV_DEFAULT_ &self->watcher);
		/* Let ev handle it.  */
		(void) self.release();
	}

};

}

namespace Ev {

Ev::Io<std::string> runcmd( std::string command
			  , std::vector<std::string> argv
			  ) {
	return Ev::Io<std::string>([command, argv
				   ]( std::function<void(std::string)> pass
				    , std::function<void(std::exception_ptr)> fail
				    ) {
		auto obj = Util::make_unique<RunCmd>( std::move(command)
						    , std::move(argv)
						    , std::move(pass)
						    , std::move(fail)
						    );
		/* Execute.  */
		RunCmd::run(std::move(obj));
	});
}

}
