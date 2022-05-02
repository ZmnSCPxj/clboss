#include"Boss/Mod/AvailableRpcCommandsAnnouncer.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/ModG/RpcProxy.hpp"
#include"Boss/Msg/Begin.hpp"
#include"Boss/Msg/AvailableRpcCommands.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Json/Out.hpp"
#include"Util/make_unique.hpp"
#include<sstream>

namespace Boss { namespace Mod {

class AvailableRpcCommandsAnnouncer::Impl {
private:
	ModG::RpcProxy rpc;
	void start(S::Bus& bus) {
		bus.subscribe<Msg::Begin
			     >([this, &bus](Msg::Begin const&) {
			return Boss::concurrent(on_begin(bus));
		});
	}

	Ev::Io<void> on_begin(S::Bus& bus) {
		return rpc.command( "help"
				  , Json::Out::empty_object()
				  ).then([&bus](Jsmn::Object res) {

			if (!res.is_object())
				return error( bus, res
					    , "Result not an object"
					    );
			if (!res.has("help"))
				return error( bus, res
					    , "No `help` field"
					    );
			auto j_help = res["help"];
			if (!j_help.is_array())
				return error( bus, j_help
					    , "`help` field is not an "
					      "array"
					    );

			auto msg = Boss::Msg::AvailableRpcCommands{};
			auto& table = msg.commands;

			auto num_commands = std::size_t(0);

			for (auto j_cmd : j_help) {
				if (!j_cmd.is_object())
					return error( bus, j_cmd
						    , "Command entry is "
						      "not an object"
						    );
				if (!j_cmd.has("command"))
					continue;
				auto j_full_usage = j_cmd["command"];
				if (!j_full_usage.is_string())
					continue;
				auto category = std::string("");
				if (j_cmd.has("category")) {
					auto j_category = j_cmd["category"];
					if (j_category.is_string())
						category = std::string(j_category);
				}

				auto full_usage = std::string(j_full_usage);
				auto command = std::string();
				auto usage = std::string();
				split_usage(command, usage, full_usage);

				table[command] = Msg::AvailableRpcCommands::Desc{
					std::move(category), std::move(usage)
				};
				++num_commands;
			}

			return Boss::log( bus, Debug
					, "AvailableRpcCommandsAnnouncer: "
					  "Node has %zu commands."
					, num_commands
					)
			     + bus.raise(msg)
			     ;
		}).catching<RpcError>([&bus](RpcError const& e) {
			return Boss::log( bus, Error
					, "AvailableRpcCommandsAnnouncer: "
					  "Command `help` failed!? "
					  "Error: %s"
					, e.error.direct_text().c_str()
					);
		});
	}

	static
	Ev::Io<void> error( S::Bus& bus
			  , Jsmn::Object const& obj
			  , char const* message
			  ) {
		return Boss::log( bus, Error
				, "AvailableRpcCommandsAnnouncer: "
				  "Unexpected result from `help`: "
				  "%s: %s"
				, message
				, obj.direct_text().c_str()
				);
	}

	static
	void split_usage( std::string& command
			, std::string& usage
			, std::string const& full_usage
			) {
		auto s_command = std::ostringstream();
		auto s_usage = std::ostringstream();
		auto input = std::istringstream(full_usage);
		while (input && !input.eof()) {
			auto c = input.get();
			if (input.eof())
				break;

			if (c == ' ') {
				while (input && !input.eof()) {
					c = input.get();
					if (input.eof())
						break;
					s_usage.put(c);
				}
				break;
			}
			s_command.put(c);
		}
		command = s_command.str();
		usage = s_usage.str();
	}

public:
	Impl(S::Bus& bus) : rpc(bus) { start(bus); }
	Impl(Impl const&) =delete;
	Impl(Impl&&) =delete;
};

AvailableRpcCommandsAnnouncer::AvailableRpcCommandsAnnouncer(AvailableRpcCommandsAnnouncer&&)
	=default;
AvailableRpcCommandsAnnouncer::~AvailableRpcCommandsAnnouncer()
	=default;

AvailableRpcCommandsAnnouncer::AvailableRpcCommandsAnnouncer(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }


}}
