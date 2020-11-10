#include"Boss/Mod/Rpc.hpp"
#include"Boss/Shutdown.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Parser.hpp"
#include"Json/Out.hpp"
#include"Net/Fd.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<algorithm>
#include<assert.h>
#include<cstdint>
#include<ev.h>
#include<fcntl.h>
#include<iterator>
#include<map>
#include<memory>
#include<poll.h>
#include<sstream>
#include<string.h>
#include<unistd.h>

namespace {

template<typename a>
std::string enstring(a const& val) {
	auto os = std::ostringstream();
	os << val;
	return os.str();
}
template<typename a>
std::string limited_enstring(a const& val) {
	auto rv = enstring(val);
	if (rv.size() > 160) {
		rv.erase(rv.begin() + 160, rv.end());
		rv += "...";
	}

	return rv;
}

}

namespace Boss { namespace Mod {

std::string RpcError::make_error_message( std::string const& command
					, Jsmn::Object const& e
					) {
	auto os = std::ostringstream();
	os << command << ": " << e;
	return os.str();
}

RpcError::RpcError( std::string command_
		  , Jsmn::Object error_
		  ) : std::runtime_error(make_error_message(command_, error_))
		    , command(command_)
		    , error(error_)
		    { }

class Rpc::Impl {
private:
	S::Bus& bus;

	Net::Fd socket;
	Jsmn::Parser parser;

	bool is_shutting_down;

	/* Next id.  */
	std::uint64_t next_id;

	/* Pending commands.  */
	struct Pending {
		std::string command;
		std::function<void(Jsmn::Object)> pass;
		std::function<void(std::exception_ptr)> fail;
	};
	std::map<std::uint64_t, Pending> pendings;

	/* Data to be written.  */
	std::vector<char> to_write;

	/* libev event on read end of RPC socket.  */
	ev_io read_event;
	ev_idle read_parse_event;
	bool read_parse_active;
	/* libev event on write end of RPC socket.  */
	std::unique_ptr<ev_io> write_event;

	/* Data we get on the read end of the RPC socket.  */
	std::string read_buffer;

	/* Call at shutdown.  */
	void shutdown() {
		is_shutting_down = true;
		ev_io_stop(EV_DEFAULT_ &read_event);
		if (write_event) {
			ev_io_stop(EV_DEFAULT_ write_event.get());
			write_event = nullptr;
		}

		auto pendings_copy = std::move(pendings);

		/* Fail everything.  */
		for (auto const& ip : pendings_copy) {
			auto const& p = ip.second;
			try {
				throw Boss::Shutdown();
			} catch (...) {
				p.fail(std::current_exception());
			}
		}

	}

	/* Process a single response.  */
	void process_response(Jsmn::Object const& resp) {
		/* Silently fail.  */
		if (!resp.is_object())
			return;
		if (!resp.has("id"))
			return;
		if (!resp["id"].is_number())
			return;

		auto id = std::uint64_t(double(resp["id"]));
		auto it = pendings.find(id);

		/* Silently fail.  */
		if (it == pendings.end())
			return;

		if (resp.has("result")) {
			auto pass = std::move(it->second.pass);
			pendings.erase(it);
			pass(resp["result"]);
		} else if (resp.has("error")) {
			auto command = std::move(it->second.command);
			auto fail = std::move(it->second.fail);
			pendings.erase(it);
			try {
				throw RpcError( std::move(command)
					      , resp["error"]
					      );
			} catch (...) {
				fail(std::current_exception());
			}
		}
	}

	/* Is the given fd *really* ready for reading/writing?  */
	static
	bool is_ready(int fd, int events) {
		auto pollarg = pollfd();
		pollarg.fd = fd;
		pollarg.events = events;
		pollarg.revents = 0;

		auto res = int();
		do {
			res = poll(&pollarg, 1, 0);
		} while (res < 0 && errno == EINTR);
		if (res < 0)
			/* Assume ready.... */
			return true;
		return (pollarg.revents & events) != 0;
	}

	/* Call when read end is ready.  */
	void on_read() {
		auto static constexpr chunk_size = std::size_t(4096);

		while (is_ready(socket.get(), POLLIN)) {
			auto offset = read_buffer.size();
			read_buffer.resize(offset + chunk_size);
			auto ptr = &*(read_buffer.begin() + offset);

			auto res = ssize_t();
			do {
				res = read(socket.get(), ptr, chunk_size);
			} while (res < 0 && errno == EINTR);
			if (res < 0 && ( errno == EWOULDBLOCK
				      || errno == EAGAIN
				       )) {
				read_buffer.resize(offset);
				break;
			}
			if (res < 0)
				throw std::runtime_error(
					std::string("Rpc: read: ") +
					strerror(errno)
				);
			if (res == 0)
				/* Unexpected end of file!  */
				throw std::runtime_error(
					"Rpc: read: unexpected end-of-file "
					"in RPC socket."
				);
			if (std::size_t(res) < chunk_size)
				read_buffer.resize(offset + res);
		}
	}
	/* Wrapper of above for libev.  */
	static
	void on_read_static(EV_P_ ev_io *e, int revents) {
		auto self = reinterpret_cast<Impl*>(e->data);
		self->on_read();
		/* Re-arm.  */
		ev_io_start(EV_A_ e);
		/* Trigger read parse event.  */
		if ( !self->read_parse_active
		  && self->read_buffer.size() != 0
		   ) {
			self->read_parse_active = true;
			ev_idle_start(EV_A_ &self->read_parse_event);
		}
	}

	/* Call when read event took some data.  */
	void on_read_parse() {
		auto responses = parser.feed(read_buffer);
		read_buffer.resize(0);
		for (auto const& r : responses)
			process_response(r);
	}
	static
	void on_read_parse_static(EV_P_ ev_idle* e, int _) {
		auto self = reinterpret_cast<Impl*>(e->data);
		ev_idle_stop(EV_A_ e);
		self->read_parse_active = false;
		self->on_read_parse();
	}

	/* Call when write end is ready.  */
	void on_write() {
		while ( is_ready(socket.get(), POLLOUT)
		     && to_write.size() != 0
		      ) {
			auto res = ssize_t();
			auto size = to_write.size();
			if (size > 512)
				size = 512;
			do {
				res = write( socket.get()
					   , &to_write[0], size
					   );
			} while (res < 0 && errno == EINTR);
			if (res < 0 && ( errno == EWOULDBLOCK
				      || errno == EAGAIN
				       ))
				break;
			if (res < 0)
				throw std::runtime_error(
					std::string("Rpc: write: ") +
					strerror(errno)
				);

			to_write.erase( to_write.begin()
				      , to_write.begin() + res
				      );
		}
		if (to_write.size() == 0) {
			if (write_event) {
				ev_io_stop(EV_DEFAULT_ write_event.get());
				write_event = nullptr;
			}
		} else {
			/* Re-arm.  */
			if (!write_event) {
				write_event = Util::make_unique<ev_io>();
				ev_io_init( write_event.get(), &on_write_static
					  , socket.get(), EV_WRITE
					  );
				write_event->data = this;
			}
			ev_io_start(EV_DEFAULT_ write_event.get());
		}
	}
	/* Wrapper of above for libev.  */
	static
	void on_write_static(EV_P_ ev_io *e, int revents) {
		auto self = reinterpret_cast<Impl*>(e->data);
		ev_io_stop(EV_A_ self->write_event.get());
		self->write_event = nullptr;
		self->on_write();
	}

public:
	Impl( S::Bus& bus_
	    , Net::Fd socket_
	    ) : bus(bus_)
	      , socket(std::move(socket_))
	      , is_shutting_down(false)
	      , next_id(0)
	      , write_event(nullptr)
	      , read_buffer("")
	      {

		{
			/* Make it non-blocking.  */
			auto flags = fcntl(socket.get(), F_GETFL);
			flags |= O_NONBLOCK;
			fcntl(socket.get(), F_SETFL, flags);
		}

		ev_io_init( &read_event, &on_read_static
			  , socket.get(), EV_READ
			  );
		read_event.data = this;
		ev_set_priority(&read_event, -1);
		ev_io_start(EV_DEFAULT_ &read_event);

		ev_idle_init(&read_parse_event, &on_read_parse_static);
		read_parse_event.data = this;
		read_parse_active = false;

		/* Listen to Boss::Shutdown events.  */
		bus.subscribe<Boss::Shutdown>([this](Boss::Shutdown const&) {
			shutdown();
			return Ev::lift();
		});
	}

	~Impl() {
		if (write_event) {
			ev_io_stop(EV_DEFAULT_ write_event.get());
			write_event = nullptr;
		}
		if (!is_shutting_down)
			shutdown();
	}

	Ev::Io<Jsmn::Object> core_command( std::string const& command
					 , Json::Out params
					 ) {
		return Ev::Io<Jsmn::Object>([=]( std::function<void(Jsmn::Object)> pass
					       , std::function<void(std::exception_ptr)> fail
					       ) {
			if (is_shutting_down) {
				try {
					throw Boss::Shutdown();
				} catch (...) {
					fail(std::current_exception());
				}
				return;
			}

			auto id = next_id++;
			auto js = Json::Out()
				.start_object()
					.field("jsonrpc", std::string("2.0"))
					.field("id", double(id))
					.field("method", command)
					.field("params", params)
				.end_object()
				.output() + "\n\n";
			std::copy( js.begin(), js.end()
				 , std::back_inserter(to_write)
				 );

			/* Add to pending.  */
			pendings[id] = Pending{ std::move(command)
					      , std::move(pass)
					      , std::move(fail)
					      };

			/* Perform the write.  */
			on_write();
		});
	}
	Ev::Io<Jsmn::Object> command( std::string const& command
				    , Json::Out params
				    ) {
		auto save = std::make_shared<Jsmn::Object>();
		auto errsave = std::make_shared<RpcError>(
			"", Jsmn::Object()
		);
		return Boss::log( bus, Debug
				, "Rpc out: %s %s"
				, command.c_str()
				, params.output().c_str()
				).then([this, command, params]() {
			return core_command(command, params);
		}).then([this, command, params, save](Jsmn::Object result) {
			*save = std::move(result);
			return Boss::log( bus, Debug
					, "Rpc in: %s %s => %s"
					, command.c_str()
					, params.output().c_str()
					, limited_enstring(*save).c_str()
					);
		}).then([save]() {
			return Ev::lift(std::move(*save));
		}).catching<RpcError>([ this
				      , command
				      , params
				      , errsave
				      ](RpcError const& e) {
			*errsave = e;
			return Boss::log( bus, Debug
					, "Rpc in: %s %s => error %s"
					, command.c_str()
					, params.output().c_str()
					, limited_enstring(errsave->error).c_str()
					).then([errsave]() {
				throw *errsave;
				/* Needed for inference.  */
				return Ev::lift(errsave->error);
			});
		});
	}
};

Rpc::Rpc( S::Bus& bus
	, Net::Fd socket
	) : pimpl(Util::make_unique<Impl>(bus, std::move(socket)))
	  { }
Rpc::Rpc(Rpc&& o) : pimpl(std::move(o.pimpl)) { }
Rpc::~Rpc() { }

Ev::Io<Jsmn::Object> Rpc::command( std::string const& command
				 , Json::Out params
				 ) {
	assert(pimpl);
	return pimpl->command(command, std::move(params));
}

}}

