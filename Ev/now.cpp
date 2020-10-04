#include"Ev/now.hpp"
#include<ev.h>

namespace Ev {

double now() {
	return ev_now(EV_DEFAULT);
}

}
