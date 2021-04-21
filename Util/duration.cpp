#include"Util/duration.hpp"
#include<math.h>
#include<sstream>
#include<utility>

namespace Util {

std::string duration(double duration) {
	auto os = std::ostringstream();

	if (duration < 0)
		duration = 0;

	if (duration < 12) {
		os << duration << " seconds";
	} else if (duration < 60) {
		os << (unsigned int) floor(duration) << " seconds";
	} else {
		auto first = true;
		auto print_unit = [ &os
				  , &first
				  ](unsigned long value, char const* unit) {
			if (first)
				first = false;
			else
				os << ", ";
			os << value << " " << unit;
		};

		auto minutes = (unsigned long) floor(duration / 60);
		auto seconds = (unsigned long) (duration - (minutes * 60));
		if (minutes > 60) {
			auto hours = minutes / 60;
			minutes -= hours * 60;

			if (hours > 24) {
				auto days = hours / 24;
				hours -= days * 24;

				if (days > 7) {
					auto weeks = (unsigned long) 0;
					auto months = (unsigned long) 0;
					auto years = (unsigned long) 0;

					if (days < 30) {
						weeks = days / 7;
					} else if (days < 90) {
						months = days / 30;
						weeks = (days - (months * 30)) / 7;
					} else if (days < 365) {
						months = days / 30;
					} else {
						years = days / 365;
						months = (days - (years * 365)) / 30;
					}

					days -= weeks * 7;
					days -= months * 30;
					days -= years * 365;

					if (years != 0)
						print_unit(years, "years");
					if (months != 0)
						print_unit(months, "months");
					if (weeks != 0)
						print_unit(weeks, "weeks");
				}
				if (days != 0)
					print_unit(days, "days");
			}

			if (hours != 0)
				print_unit(hours, "hours");
		}
		if (minutes != 0)
			print_unit(minutes, "minutes");
		if (seconds != 0)
			print_unit(seconds, "seconds");
	}

	return std::move(os).str();
}

}
