#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([basicsecure], [1.0], [ZmnSCPxj@protonmail.com])
AC_CONFIG_AUX_DIR([auxdir])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([basicsecure.h])
AC_CONFIG_HEADERS([basicsecure_config.h])
AC_CONFIG_MACRO_DIRS([m4])
LT_INIT

# Checks for programs.
AC_PROG_CC

# Check for C11.
AX_CHECK_COMPILE_FLAG([-std=c11], [
	CFLAGS="$CFLAGS -std=c11"
])

############################################################################
# basicsecure_clear
############################################################################

# Check for memset_s.
AC_MSG_CHECKING([for memset_s])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#define _DEFAULT_SOURCE
#include<string.h>
]], [[
	unsigned char array[42];
	(void) memset_s(array, 0, sizeof(array));
]])], [ # then
	AC_DEFINE([HAVE_MEMSET_S], [1],
		  [Define to 1 if you have a memset_s function.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

# Check for explicit_bzero.
AC_MSG_CHECKING([for explicit_bzero])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#define _DEFAULT_SOURCE
#include<string.h>
]], [[
	unsigned char array[42];
	(void) explicit_bzero(array, sizeof(array));
]])], [ # then
	AC_DEFINE([HAVE_EXPLICIT_BZERO], [1],
		  [Define to 1 if you have a explicit_bzero function.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

# Check for explicit_memset.
AC_MSG_CHECKING([for explicit_memset])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#define _DEFAULT_SOURCE
#include<string.h>
]], [[
	unsigned char array[42];
	(void) explicit_memset(array, 0, sizeof(array));
]])], [ # then
	AC_DEFINE([HAVE_EXPLICIT_MEMSET], [1],
		  [Define to 1 if you have a explicit_memset function.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

# Check for SecureZeroMemory.
AC_MSG_CHECKING([for SecureZeroMemory])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include<windows.h>
#include<wincrypt.h>
]], [[
	unsigned char array[42];
	(void) SecureZeroMemory(array, sizeof(array));
]])], [ # then
	AC_DEFINE([HAVE_WINDOWS_SECUREZEROMEMORY], [1],
		  [Define to 1 if you have a SecureZeroMemory function.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

# Check if generic asm; works.
AC_MSG_CHECKING([if plain asm; works])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
]], [[
	asm;
]])], [ # then
	AC_DEFINE([HAVE_PLAIN_ASM_CLEAR], [1],
		  [Define to 1 if plain asm; declaration works.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

# Check if GCC ASM works.
AC_MSG_CHECKING([if GCC __asm__ works])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include<stddef.h>
]], [[
	unsigned char array[42];
	void* p = array;
	size_t len = sizeof(array);
	volatile unsigned char* volatile vp =
		(volatile unsigned char* volatile) p;
	size_t i;
	for (i = 0; i < len; ++i)
		vp[i] = 0;
	__asm__ __volatile__ ("" : : "r"(p), "r"(vp) : "memory");
]])], [ # then
	AC_DEFINE([HAVE_GCC_ASM_CLEAR], [1],
		  [Define to 1 if GCC-style __asm__ declaration works.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

# Check if __attribute__((weak)) works.
AC_MSG_CHECKING([if __attribute__((weak)) works])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include<stddef.h>
__attribute__((weak))
void nonexistent();
]], [[
	if (nonexistent)
		nonexistent();
]])], [ # then
	AC_DEFINE([HAVE_ATTRIBUTE_WEAK], [1],
		  [Define to 1 if __attribute__((weak)) works.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

############################################################################
# basicsecure_rand
############################################################################

# Check for getentropy.
AC_MSG_CHECKING([for getentropy])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#define _DEFAULT_SOURCE
#include<unistd.h>
]], [[
	unsigned char array[42];
	getentropy(array, sizeof(array));
]])], [ # then
	AC_DEFINE([HAVE_UNISTD_GETENTROPY], [1],
		  [Define to 1 if you have a working getentropy in unistd.h.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

# Check for Linux getrandom.
AC_MSG_CHECKING([for Linux getrandom])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#define _DEFAULT_SOURCE
#include<sys/syscall.h>
#include<unistd.h>
#ifndef SYS_getrandom
# error "No Linux getrandom."
#endif
#define getrandom(buf, size, flags) \
	syscall(SYS_getrandom, (buf), (int) (size), (flags))
]], [[
	unsigned char array[42];
	getrandom(array, sizeof(array), 0);
]])], [ # then
	AC_DEFINE([HAVE_LINUX_GETRANDOM], [1],
		  [Define to 1 if you have a working Linux getrandom syscall.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

# Check for BSD getrandom.
AC_MSG_CHECKING([for BSD getrandom])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#define _DEFAULT_SOURCE
#include<sys/random.h>
#include<sys/param.h>
#ifndef __FreeBSD__
# error "Not FreeBSD."
#endif
#ifndef __FreeBSD_version
# error "No FreeBSD version."
#endif
#if __FreeBSD_version < 1200000
# error "Old FreeBSD version."
#endif
]], [[
	unsigned char array[42];
	getrandom(array, sizeof(array), 0);
]])], [ # then
	AC_DEFINE([HAVE_BSD_GETRANDOM], [1],
		  [Define to 1 if you have a working BSD getrandom syscall.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

# Check for RtlGenRandom.
AC_MSG_CHECKING([for Windows RtlGenRandom])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include<windows.h>
#define RtlGenRandom SystemFunction036
#ifdef __cplusplus
extern "C"
#endif
BOOLEAN NTAPI RtlGenRandom(PVOID s, ULONG n);
]], [[
	unsigned char array[42];
	(void) RtlGenRandom((PVOID) array, (ULONG) sizeof(array));
]])], [ # then
	AC_DEFINE([HAVE_WINDOWS_RTLGENRANDOM], [1],
		  [Define to 1 if you have a working Windows RtlGenRandom.])
	AC_MSG_RESULT([yes])
], [ # else
	AC_MSG_RESULT([no])
])

############################################################################

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
