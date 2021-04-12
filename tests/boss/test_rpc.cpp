#undef NDEBUG
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Shutdown.hpp"
#include"Ev/Io.hpp"
#include"Ev/concurrent.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Net/Fd.hpp"
#include"S/Bus.hpp"
#include<assert.h>
#include<cstdint>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>

namespace {

/* Simple server.  */
class SimpleServer {
private:
	Net::Fd socket;

	Ev::Io<void> writeloop(std::string to_write) {
		/* Yield to let other greenthreads run, then
		 * keep trying to write.  */
		return Ev::yield().then([this, to_write]() {
			auto res = write( socket.get()
					, to_write.c_str(), to_write.size()
					);
			if (res < 0 && ( errno == EWOULDBLOCK
				      || errno == EAGAIN
				       ))
				/* Loop.  */
				return writeloop(to_write);
			assert(size_t(res) == to_write.size());
			return Ev::yield();
		});
	}

	/* Slurp any incoming data.  */
	Ev::Io<void> slurp() {
		return Ev::yield().then([this]() {
			char buf[256];
			auto first = true;
			for (;;) {
				auto res = read( socket.get()
					       , buf, sizeof(buf)
					       );
				if (res < 0 && ( errno == EWOULDBLOCK
					      || errno == EAGAIN
					       )) {
					if (first)
						/* No data from client yet.  */
						return slurp();
					else
						break;
				}
				assert(res >= 0);
				first = false;
			}
			return Ev::yield();
		});
	}

public:
	explicit
	SimpleServer(Net::Fd socket_) : socket(std::move(socket_)) {
		auto flags = fcntl(socket.get(), F_GETFL);
		flags |= O_NONBLOCK;
		fcntl(socket.get(), F_SETFL, flags);
	}
	SimpleServer(SimpleServer&& o) : socket(std::move(o.socket)) { }

	Ev::Io<void> respond(std::uint64_t id) {
		return Ev::yield().then([this]() {
			/* Slurp any data.  */
			return slurp();
		}).then([this, id]() {
			/* Build response.  */
			auto js = Json::Out()
				.start_object()
					.field("jsonrpc", std::string("2.0"))
					.field("id", double(id))
					.field("result", Json::Out().start_object().end_object())
				.end_object()
				.output()
				;
			return writeloop(js);
		});
	}
	Ev::Io<void> error( std::uint64_t id
			  , int code
			  , std::string message
			  ) {
		return Ev::yield().then([this]() {
			/* Slurp any data.  */
			return slurp();
		}).then([this, id, code, message]() {
			/* Build response.  */
			auto err = Json::Out()
				.start_object()
					.field("code", double(code))
					.field("message", message)
				.end_object()
				;
			auto js = Json::Out()
				.start_object()
					.field("jsonrpc", std::string("2.0"))
					.field("id", double(id))
					.field("error", err)
				.end_object()
				.output()
				;
			return writeloop(js);
		});
	}
};

}

int main() {
	auto bus = S::Bus();

	int sockets[2];
	auto res = socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);
	assert(res >= 0);
	auto server_socket = Net::Fd(sockets[0]);
	auto client_socket = Net::Fd(sockets[1]);

	auto server = SimpleServer(std::move(server_socket));
	auto client = Boss::Mod::Rpc(bus, std::move(client_socket));

	auto succeed_flag = false;
	auto errored_flag = false;
	auto shutdown_flag = false;

	/* Server responds with success, then failure, then shuts down.  */
	auto server_code = Ev::lift().then([&]() {
		return server.respond(0);
	}).then([&]() {
		return server.error(1, -32600, "Some error");
	}).then([&]() {
		return bus.raise(Boss::Shutdown());
	});
	/* Client makes three attempts, the first succeeds, the second
	 * fails, the third with a cancellation of commands.  */
	auto client_code = Ev::lift().then([&]() {
		auto params = Json::Out();
		auto obj = params.start_object();
		{
			auto arr = obj.start_array("arr");
			for (auto i = size_t(0); i < 10000; ++i)
				arr.entry((double)i);
			arr.end_array();
		}
		obj.end_object();
		return client.command("c1", params);
	}).then([&](Jsmn::Object ignored_result) {
		succeed_flag = true;
		return client.command("c2", Json::Out::empty_object())
				.catching<Boss::Mod::RpcError>([&](Boss::Mod::RpcError const& e) {
			errored_flag = true;
			return Ev::lift(Jsmn::Object());
		});
	}).then([&](Jsmn::Object ignored_result) {
		return client.command("c3", Json::Out::empty_object())
				.catching<Boss::Shutdown>([&](Boss::Shutdown const& e) {
			shutdown_flag = true;
			return Ev::lift(Jsmn::Object());
		});
	}).then([](Jsmn::Object ignored_result) {
		return Ev::lift(0);
	});

	/* Launch code.  */
	auto code = Ev::lift().then([&]() {
		return Ev::concurrent(server_code);
	}).then([&]() {
		return client_code;
	});

	auto ec = Ev::start(code);

	assert(succeed_flag);
	assert(errored_flag);
	assert(shutdown_flag);

	return ec;
}
