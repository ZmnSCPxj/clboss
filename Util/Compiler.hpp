#ifndef COMPILER_H_
#define COMPILER_H_

#if defined(__GNUC__) && (__GNUC__ >= 13)
// We need a GCC patch here due the following bug
// <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107134>
#include<cstdint>
#endif

#endif // COMPILER_H_
