#ifndef BITCOIN_SHA256_CRYPTO_COMMON_H
#define BITCOIN_SHA256_CRYPTO_COMMON_H

/* Created by ZmnSCPxj as a minimal compatibility shim.  */

#include<stdint.h>

namespace {

void WriteBE32(unsigned char* ptr, uint32_t x) {
	*(ptr++) = uint8_t((x >> 24) & 0xFF);
	*(ptr++) = uint8_t((x >> 16) & 0xFF);
	*(ptr++) = uint8_t((x >> 8) & 0xFF);
	*(ptr++) = uint8_t((x >> 0) & 0xFF);
}
void WriteBE64(unsigned char* ptr, uint64_t x) {
	*(ptr++) = uint8_t((x >> 56) & 0xFF);
	*(ptr++) = uint8_t((x >> 48) & 0xFF);
	*(ptr++) = uint8_t((x >> 40) & 0xFF);
	*(ptr++) = uint8_t((x >> 32) & 0xFF);
	*(ptr++) = uint8_t((x >> 24) & 0xFF);
	*(ptr++) = uint8_t((x >> 16) & 0xFF);
	*(ptr++) = uint8_t((x >> 8) & 0xFF);
	*(ptr++) = uint8_t((x >> 0) & 0xFF);
}
uint32_t ReadBE32(unsigned char const* ptr) {
	return (uint32_t(ptr[0]) << 24)
	     | (uint32_t(ptr[1]) << 16)
	     | (uint32_t(ptr[2]) << 8)
	     | (uint32_t(ptr[3]) << 0)
	     ;
}

}

#endif /* !defined(BITCOIN_SHA256_CRYPTO_COMMON_H) */
