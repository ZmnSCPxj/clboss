#undef NDEBUG
#include"Boss/Mod/ChannelCreator/RearrangerBySize.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include<assert.h>
#include<map>

namespace {

auto const A = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000000");
auto const B = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001");
auto const C = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000002");
auto const D = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000003");
auto const E = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000004");

}

int main() {
	auto caps = std::map<Ln::NodeId, Ln::Amount>();
	auto dowser = [&caps](Ln::NodeId a, Ln::NodeId _) {
		return Ev::lift(caps[a]);
	};
	auto mut = Boss::Mod::ChannelCreator::RearrangerBySize(dowser);

	typedef std::pair<Ln::NodeId, Ln::NodeId> Proposal;
	typedef std::vector<Proposal> ProposalsVector;

	auto code = Ev::lift().then([&]() {

		/* No change.  */
		caps[A] = Ln::Amount::btc(1.0);
		caps[B] = Ln::Amount::btc(0.9);
		caps[C] = Ln::Amount::btc(0.8);
		return mut.rearrange_by_size({{A, A}, {B, B}, {C, C}});
	}).then([&](ProposalsVector actual) {
		auto expected = ProposalsVector{{A, A}, {B, B}, {C, C}};
		assert(actual == expected);

		/* With group_by = 3, D, despite being much higher,
		 * should still be last.  */
		caps[A] = Ln::Amount::btc(1.0);
		caps[B] = Ln::Amount::btc(0.9);
		caps[C] = Ln::Amount::btc(0.8);
		caps[D] = Ln::Amount::btc(21000000.0);
		return mut.rearrange_by_size({{A, A}, {B, B}, {C, C}, {D, D}});
	}).then([&](ProposalsVector actual) {
		auto expected = ProposalsVector{{A, A}, {B, B}, {C, C}, {D, D}};
		assert(actual == expected);

		/* Check 1 proposal.  */
		caps[A] = Ln::Amount::btc(1.0);
		return mut.rearrange_by_size({{A, A}});
	}).then([&](ProposalsVector actual) {
		auto expected = ProposalsVector{{A, A}};
		assert(actual == expected);

		/* Check 2 proposals with the second one being better.  */
		caps[A] = Ln::Amount::btc(1.0);
		caps[B] = Ln::Amount::btc(1.2);
		return mut.rearrange_by_size({{A, A}, {B, B}});
	}).then([&](ProposalsVector actual) {
		auto expected = ProposalsVector{{B, B}, {A, A}};
		assert(actual == expected);

		/* Check multiple rearrangement case.
		 * The below assumes group_by = 3.  */
		caps[A] = Ln::Amount::btc(1.0);
		caps[B] = Ln::Amount::btc(2.0);
		caps[C] = Ln::Amount::btc(3.0);
		caps[D] = Ln::Amount::btc(4.0);
		caps[E] = Ln::Amount::btc(5.0);
		return mut.rearrange_by_size({{A, A}, {B, B}, {C, C}, {D, D}, {E, E}});
	}).then([&](ProposalsVector actual) {
		auto expected = ProposalsVector{{C, C}, {B, B}, {A, A}, {E, E}, {D, D}};
		assert(actual == expected);

		return Ev::lift(0);
	});

	return Ev::start(code);
}
