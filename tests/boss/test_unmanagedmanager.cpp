#undef NDEBUG
#include"Boss/Mod/UnmanagedManager.hpp"
#include"Boss/Msg/CommandFail.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/ProvideUnmanagement.hpp"
#include"Boss/Msg/SolicitUnmanagement.hpp"
#include"Ev/start.hpp"
#include"Ln/CommandId.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include<assert.h>
#include<functional>
#include<set>
#include<sstream>

namespace {

auto const A = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000000");
auto const B = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001");
auto const C = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000002");

}

int main() {
	using Boss::Msg::CommandFail;
	using Boss::Msg::CommandRequest;
	using Boss::Msg::CommandResponse;
	using Boss::Msg::DbResource;
	using Boss::Msg::ProvideUnmanagement;
	using Boss::Msg::SolicitUnmanagement;

	auto bus = S::Bus();
	auto db = Sqlite3::Db(":memory:");

	auto um = Boss::Mod::UnmanagedManager(bus);

	auto example1 = std::set<Ln::NodeId>();
	auto example2 = std::set<Ln::NodeId>();
	auto example3 = std::set<Ln::NodeId>();
	/* Provide for the above.  */
	auto make_func_for = [](std::set<Ln::NodeId>& set) {
		return [&set](Ln::NodeId const& node, bool unmanaged) {
			if (unmanaged)
				set.insert(node);
			else {
				auto it = set.find(node);
				if (it != set.end())
					set.erase(it);
			}
			return Ev::lift();
		};
	};
	auto solicited = false;
	bus.subscribe< SolicitUnmanagement
		     >([&](SolicitUnmanagement const& _) {
		solicited = true;
		return bus.raise(ProvideUnmanagement{
			"example1", make_func_for(example1)
		}) + bus.raise(ProvideUnmanagement{
			"example2", make_func_for(example2)
		}) + bus.raise(ProvideUnmanagement{
			"example3", make_func_for(example3)
		});
	});

	/* Simulate clboss-unmanage commands.  */
	auto req_id = std::uint64_t();
	auto rsp_id = Ln::CommandId();
	auto rsp = false;
	auto wait_for_response = std::function<Ev::Io<bool>(std::uint64_t)>();
	wait_for_response = [&](std::uint64_t expected_id) {
		return Ev::yield().then([expected_id, &rsp_id, &rsp, &wait_for_response]() {
			auto ok = false;
			rsp_id.cmatch([&](std::uint64_t nid) {
				ok = (expected_id == nid);
			}, [&](std::string const& _) {
				ok = false;
			});
			if (!ok)
				return wait_for_response(expected_id);
			return Ev::lift(rsp);
		});
	};
	bus.subscribe< CommandFail
		     >([&](CommandFail const& m) {
		rsp_id = m.id;
		rsp = false;
		return Ev::yield();
	});
	bus.subscribe< CommandResponse
		     >([&](CommandResponse const& m) {
		rsp_id = m.id;
		rsp = true;
		return Ev::yield();
	});
	/* Return false if errored, true if success.  */
	auto clboss_unmanage = [&](Ln::NodeId const& nodeid, std::string const& tags) {
		++req_id;
		auto my_id = req_id;

		auto parms_s = std::string();
		/* Test both array and object forms.  */
		if ((my_id % 2) == 0)
			/* array. */
			parms_s = std::string("[ \"")
				+ std::string(nodeid)
				+ std::string("\", \"")
				+ tags
				+ std::string("\"]")
				;
		else
			/* object.  */
			parms_s = std::string("{ \"tags\": \"")
				+ tags
				+ std::string("\", \"nodeid\": \"")
				+ std::string(nodeid)
				+ std::string("\"}")
				;
		auto parms = Jsmn::Object();
		auto is = std::istringstream(std::move(parms_s));
		is >> parms;

		return bus.raise(CommandRequest{
			"clboss-unmanage", std::move(parms), Ln::CommandId::left(my_id)
		}).then([&wait_for_response, my_id]() {
			return wait_for_response(my_id);
		});
	};

	auto code = Ev::lift().then([&]() {
		
		/* The initial db-resource message should trigger solicitation.  */
		return bus.raise(DbResource{db});
	}).then([&]() {
		assert(solicited);

		return Ev::yield();
	}).then([&]() {

		/* A should be added to example1.  */
		return clboss_unmanage(A, "example1");
	}).then([&](bool pass) {
		assert(pass);
		assert(example1.count(A) == 1);
		assert(example1.size() == 1);
		assert(example2.size() == 0);
		assert(example3.size() == 0);

		/* Nonexistent tags should just fail without changing anything.  */
		return clboss_unmanage(A, "nonexistent,example1,example3");
	}).then([&](bool pass) {
		assert(!pass);
		assert(example1.count(A) == 1);
		assert(example1.size() == 1);
		assert(example2.size() == 0);
		assert(example3.size() == 0);

		return clboss_unmanage(B, "example1,example3");
	}).then([&](bool pass) {
		assert(pass);
		/* A should still be in example1.  */
		assert(example1.count(A) == 1);
		assert(example1.count(B) == 1);
		assert(example1.size() == 2);
		assert(example2.size() == 0);
		assert(example3.count(B) == 1);
		assert(example3.size() == 1);

		return clboss_unmanage(C, "example2,example3");
	}).then([&](bool pass) {
		assert(pass);
		/* A and B should not change.  */
		assert(example1.count(A) == 1);
		assert(example1.count(B) == 1);
		assert(example1.size() == 2);
		assert(example2.count(C) == 1);
		assert(example2.size() == 1);
		assert(example3.count(B) == 1);
		assert(example3.count(C) == 1);
		assert(example3.size() == 2);

		/* Remanage A.  */
		return clboss_unmanage(A, "");
	}).then([&](bool pass) {
		assert(pass);
		assert(example1.count(A) == 0);
		assert(example2.count(A) == 0);
		assert(example3.count(A) == 0);

		/* Change management of B.  */
		return clboss_unmanage(B, "example1,example2");
	}).then([&](bool pass) {
		assert(pass);
		assert(example1.count(B) == 1);
		assert(example2.count(B) == 1);
		assert(example3.count(B) == 0);

		return Ev::lift(0);
	});

	return Ev::start(code);
}
