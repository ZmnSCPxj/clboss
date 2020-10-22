#ifndef BOSS_MOD_MOVEFUNDSCOMMAND_HPP
#define BOSS_MOD_MOVEFUNDSCOMMAND_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::MoveFundsCommand
 *
 * @brief Provides a debugging `clboss-movefunds` command.
 * This command is intended for developer debugging and will
 * be removed in the future.
 */
class MoveFundsCommand {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	MoveFundsCommand() =delete;

	MoveFundsCommand(MoveFundsCommand&&);
	~MoveFundsCommand();

	explicit
	MoveFundsCommand(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_MOVEFUNDSCOMMAND_HPP) */
