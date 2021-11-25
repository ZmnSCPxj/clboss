#ifndef BOSS_MOD_FUNDSMOVER_ATTEMPTER_HPP
#define BOSS_MOD_FUNDSMOVER_ATTEMPTER_HPP

#include<cstdint>
#include<memory>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace Ln { class Amount; }
namespace Ln { class NodeId; }
namespace Ln { class Preimage; }
namespace Ln { class Scid; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace FundsMover {

/** class Boss::Mod::FundsMover::Attempter
 *
 * @brief Makes an attempt to move a specific amount of funds
 * from one channel to another.
 *
 * @desc This object is dynamically created during runtime.
 */
class Attempter {
private:
	class Impl;
	std::shared_ptr<Impl> pimpl;

	Attempter() =default;

public:
	/* Return true if succeed, false if fail.  */
	static
	Ev::Io<bool>
	run( S::Bus& bus
	   , Boss::Mod::Rpc& rpc
	   , Ln::NodeId self
	   /* The preimage+payment_secret should have been pre-arranged to
	    * be claimed.  */
	   , Ln::Preimage preimage
	   , Ln::Preimage payment_secret
	   /* Either source or destination can equal self, but not both.
	    * If source == self, this is "do not care", i.e. we will find
	    * any source connected to our node.
	    * Similarly, if destination == self, this means we will find
	    * any destination connected to our node.
	    */
	   , Ln::NodeId source
	   , Ln::NodeId destination
	   , Ln::Amount amount
	   /* Budgeting information.
	    * Attempter will deduct from this fee budget when it sends out an
	    * attempt, and will return the deducted fee budget if an attempt
	    * fails.
	    */
	   , std::shared_ptr<Ln::Amount> fee_budget
	   /* Remaining amount that still needs to be sent out.
	    * Attempter will deduct from this remaining amount when it sends
	    * out an attempt, and will return the deducted amount if an
	    * attempt fails.
	    * This is used to rescale the fee limit of this attempt when
	    * this attempt is less than the remaining amount to send.
	    */
	   , std::shared_ptr<Ln::Amount> remaining_amount
	   /* Details of the channel from destination to us.
	    * Ignored if destination == self.
	    */
	   , Ln::Scid last_scid
	   , Ln::Amount base_fee
	   , std::uint32_t proportional_fee
	   , std::uint32_t cltv_delta
	   /* The channel from us to source.
	    * Ignored if source == self.
	    */
	   , Ln::Scid first_scid
	   );
};

}}}

#endif /* !defined(BOSS_MOD_FUNDSMOVER_ATTEMPTER_HPP) */
