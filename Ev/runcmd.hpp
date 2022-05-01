#ifndef EV_RUNCMD_HPP
#define EV_RUNCMD_HPP

#include<string>
#include<vector>

namespace Ev { template<typename a> class Io; }

namespace Ev {

/** Ev::runcmd
 *
 * @brief run the given command, and returns its
 * output as a string.
 *
 * @desc This action does not return until the
 * given command closes its stdout, which should
 * normally happen when it terminates.
 * All output is returned in the string.
 * Its input is redirected from `/dev/null`.
 *
 * stderr may be captured in the same pipe as
 * stdout, or may be preserved to be the same
 * as in the current process.
 */
Ev::Io<std::string> runcmd( std::string command
			  , std::vector<std::string> argv
			  , bool capture_stderr = false
			  );

}

#endif /* !defined(EV_RUNCMD_HPP) */
