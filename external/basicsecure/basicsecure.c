/* This library was written by ZmnSCPxj and is placed in the public domain.  */
#define _DEFAULT_SOURCE
#include<stddef.h>
#include<stdio.h>
#include<stdlib.h>
#include"basicsecure.h"

#ifdef HAVE_CONFIG_H
#include"basicsecure_config.h"
#endif

int basicsecure_eq(void const* p1, void const* p2, unsigned long size) {
	unsigned char* cp1 = (unsigned char*) p1;
	unsigned char* cp2 = (unsigned char*) p2;

	unsigned char v = 0;
	while (size != 0) {
		v |= (*cp1) ^ (*cp2);

		++cp1;
		++cp2;
		--size;
	}

	return (v == 0);
}

/*-----------------------------------------------------------------------------
basicsecure_clear
-----------------------------------------------------------------------------*/

#if defined(HAVE_GCC_ASM_CLEAR)
static void basicsecure_clear_guard_function(void* p, unsigned long size) {
	__asm__ __volatile__ ("" : : "r"(p) : "memory");
	(void) size;
}
#elif defined(HAVE_PLAIN_ASM_CLEAR)
static void basicsecure_clear_guard_function(void* p, unsigned long size) {
	asm;
	(void) p;
	(void) size;
}
#elif defined(HAVE_ATTRIBUTE_WEAK)
__attribute__((weak))
void basicsecure_clear_guard_function_dummy(void* p, unsigned long size);

static void basicsecure_clear_guard_function(void* p, unsigned long size) {
	/* This "should" fool link-time optimization into worrying that
	 * the function below might actually matter.  */
	if (basicsecure_clear_guard_function_dummy)
		basicsecure_clear_guard_function_dummy(p, size);
}
#else
/* Fallback.  */
static void dummy_clear_guard(void* p, unsigned long size) {
	(void) p;
	(void) size;
}
/* Not static, must be global, so that the compiler worries it might be set
 * outside this compilation unit.
 * Does not work against link-time optimization, you need __attribute__((weak))
 * for that.
 */
void (*basicsecure_clear_guard_function)(void*, unsigned long) = &dummy_clear_guard;
#endif

#if defined(HAVE_MEMSET_S)
#include<string.h>
void basicsecure_clear(void* p, unsigned long size) {
	(void) memset_s(p, 0, size);
	basicsecure_clear_guard_function(p, size);
}
#elif defined(HAVE_EXPLICIT_BZERO)
#include<string.h>
void basicsecure_clear(void* p, unsigned long size) {
	(void) explicit_bzero(p, size);
	basicsecure_clear_guard_function(p, size);
}
#elif defined(HAVE_EXPLICIT_MEMSET)
#include<string.h>
void basicsecure_clear(void* p, unsigned long size) {
	(void) explicit_memset(p, 0, size);
	basicsecure_clear_guard_function(p, size);
}
#elif defined(HAVE_WINDOWS_SECUREZEROMEMORY)
#include<windows.h>
#include<wincrypt.h>
void basicsecure_clear(void* p, unsigned long size) {
	(void) SecureZeroMemory(p, size);
	basicsecure_clear_guard_function(p, size);
}
#else

/* Fallback.  */
void basicsecure_clear(void* p, unsigned long size) {
	volatile unsigned char* volatile vp = (volatile unsigned char* volatile) p;
	size_t i;

	for(i = 0; i < size; ++i)
		vp[i] = 0;
#if defined(HAVE_INLINE_ASM_CLEAR)
	__asm__ __volatile__ ("" : : "r"(p), "r"(vp) : "memory");
#endif
	basicsecure_clear_guard_function(p, size);
	basicsecure_clear_guard_function((void*) vp, size);
}

#endif

/*-----------------------------------------------------------------------------
basicsecure_rand
-----------------------------------------------------------------------------*/

/* basicsecure_rand_core returns 0 if it fails, non-0 if it succeeds.  */

#if defined(HAVE_WINDOWS_RTLGENRANDOM)
#include<inwdows.h>
#define RtlGenRandom SystemFunction036
#ifdef __cplusplus
extern "C"
#endif
BOOLEAN NTAPI RtlGenRandom(PVOID s, ULONG n);
static
int basicsecure_rand_core(void* p, unsigned long size) {
	return !!RtlGenRandom((PVOID) p, (ULONG) size);
}
#else

#if defined(HAVE_UNISTD_GETENTROPY)
#include<unistd.h>
#define HAVE_GETENTROPY 1
#elif defined(HAVE_LINUX_GETRANDOM)
#define HAVE_GETENTROPY 1
#include<sys/syscall.h>
#define getentropy(buf, size) \
	syscall(SYS_getrandom, (buf), (int) (size), 0)
#elif defined(HAVE_BSD_GETRANDOM)
#define HAVE_GETENTROPY 1
#include<sys/param.h>
#include<sys/random.h>
#define getentropy(buf, size) \
	getrandom((buf), (size), 0)
#endif

#if defined(HAVE_GETENTROPY)
#include<errno.h>
static int getrandom_works = 1;
/* Cannot reliably get more than 256 bytes at a time from
 * getentropy.
 * getrandom can get more but could return less than the
 * specified number of bytes if greater than 512, so use
 * the 256-limit anyway.
 */
static
int basicsecure_rand_getrandom_max256(void* p, unsigned long size) {
	if (!getrandom_works)
		return 0;
	int res;
	do {
		res = getentropy(p, size);
	} while ( (res < 0)
	       && ( (errno == EINTR)
		 || (errno == EAGAIN)
		 || (errno == EWOULDBLOCK)
		  ));
	if (res < 0) {
		getrandom_works = 0;
		return 0;
	}
	return 1;
}
static
int basicsecure_rand_getrandom(void* p, unsigned long size) {
	if (!getrandom_works)
		return 0;
	while (size >= 256) {
		if (!basicsecure_rand_getrandom_max256(p, 256))
			return 0;
		p += 256;
		size -= 256;
	}
	if ((size > 0) && !basicsecure_rand_getrandom_max256(p, size))
		return 0;
	return 1;
}
#endif

#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

static
int basicsecure_rand_core(void* p, unsigned long size) {
	int fd;
	int cres;

	errno = 0;

#if defined(HAVE_GETRANDOM)
	if (basicsecure_rand_getrandom(p, size))
		return 1;
#endif

	/* Fallback.  */
	do {
		fd = open("/dev/urandom", O_RDONLY);
	} while (fd < 0 && (errno == EINTR));
	if (fd < 0)
		return 0;

	while (size > 0) {
		ssize_t res;
		do {
			res = read(fd, p, size);
		} while (res < 0 && (errno == EINTR));
		if (res < 0) {
			int saved_errno = errno;
			do {
				cres = close(fd);
			} while (cres < 0 && (errno == EINTR));
			errno = saved_errno;
			return 0;
		}

		p += res;
		size -= res;
	}

	do {
		cres = close(fd);
	} while (cres < 0 && (errno == EINTR));
	errno = 0;
	return 1;
}

#endif

void basicsecure_rand(void* p, unsigned long size, char const* comment) {
	if (!basicsecure_rand_core(p, size)) {
		perror(comment);
		abort();
	}
}

