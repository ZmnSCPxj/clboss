#ifndef COMPILER_H_
#define COMPILER_H_

#ifdef __GNUC__
#include <features.h>
// We need a GCC patch here due the following bug
// <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107134>
#if __GNUC_PREREQ(13,0)
#include<cstdint>
#endif
#endif


#endif // COMPILER_H_
