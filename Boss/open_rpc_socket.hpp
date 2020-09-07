#ifndef BOSS_OPEN_RPC_SOCKET_HPP
#define BOSS_OPEN_RPC_SOCKET_HPP

#include<string>

namespace Net { class Fd; }

namespace Boss {

/** Boss::open_rpc_socket()
 *
 * @brief changes to the given directory, and
 * attempts to open the given socket.
 *
 * @desc Function to change to the given directory
 * and open the given socket.
 * This is a separate function in order to let it
 * be replaced with a different function for
 * testing.
 */
Net::Fd open_rpc_socket( std::string const& lightning_dir = "."
		       , std::string const& rpc_file = "lightning-rpc"
		       );

}

#endif /* !defined(BOSS_OPEN_RPC_SOCKET_HPP) */
