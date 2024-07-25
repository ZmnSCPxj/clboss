#ifndef UTIL_BACKTRACE_EXCEPTION_HPP
#define UTIL_BACKTRACE_EXCEPTION_HPP

#include <array>
#include <execinfo.h>
#include <iomanip>
#include <memory>
#include <sstream>
#include <vector>
#include <string.h>

namespace Util {

/** class Util::BacktraceException<E>
 *
 * @brief A wrapper for an exception E which additionally stores a
 * backtrace when it is constructed.  The backtrace formatting is
 * deferred until `what()` is called on the handled exception.
 */

struct PcloseDeleter {
    void operator()(FILE* fp) const {
        if (fp) {
            pclose(fp);
        }
    }
};

template <typename T>
class BacktraceException : public T {
public:
	template <typename... Args>
	BacktraceException(Args&&... args)
		: T(std::forward<Args>(args)...), full_message_(T::what()) {
		capture_backtrace();
	}

	const char* what() const noexcept override {
		if (!formatted_) {
			formatted_ = true;
			full_message_ = create_full_message(T::what());
		}
		return full_message_.c_str();
	}

private:
	static constexpr size_t MAX_FRAMES = 100;
	mutable bool formatted_ = false;
	mutable std::string full_message_;
	mutable void* backtrace_addresses_[MAX_FRAMES];
	mutable size_t stack_depth;

	void capture_backtrace() {
		memset(backtrace_addresses_, 0, sizeof(backtrace_addresses_));
		stack_depth = backtrace(backtrace_addresses_, sizeof(backtrace_addresses_) / sizeof(void*));
	}

	std::string format_backtrace() const {
		char** symbols = backtrace_symbols(backtrace_addresses_, stack_depth);
		std::ostringstream oss;
		for (size_t i = 0; i < stack_depth; ++i) {
			oss << '#' << std::left << std::setfill(' ') << std::setw(2) << i << ' ';
			auto line = addr2line(backtrace_addresses_[i]);
			if (line.find("??") != std::string::npos) {
				// If addr2line doesn't find a good
				// answer use basic information
				// instead.
				oss << symbols[i] << std::endl;
			} else {
				oss << line;
			}
		}
		free(symbols);
		return oss.str();
	}

	// Unfortunately there is no simple way to get a high quality
	// backtrace using in-process libraries.  Instead for now we
	// popen an addr2line process and use it's output.
	std::string addr2line(void* addr) const {
		char cmd[512];
		snprintf(cmd, sizeof(cmd),
			 "addr2line -C -f -p -e %s %p", program_invocation_name, addr);

		std::array<char, 128> buffer;
		std::string result;
		std::unique_ptr<FILE, PcloseDeleter> pipe(popen(cmd, "r"));

		if (!pipe) {
			return " -- error: unable to open addr2line";
		}

		while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
			result += buffer.data();
		}

		return result;
	}

	std::string create_full_message(const std::string& message) const {
		std::ostringstream oss;
		oss << message << "\nBacktrace:\n" << format_backtrace();
		return oss.str();
	}
};

} // namespace Util

#endif /* UTIL_BACKTRACE_EXCEPTION_HPP */
