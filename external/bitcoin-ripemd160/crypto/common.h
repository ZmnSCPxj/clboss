/* This code was replaced by ZmnSCPxj for the CLBOSS project.  */
#include<stdint.h>

#include"Util/Str.hpp"
#include<iostream>

namespace {

void WriteLE64(unsigned char* ptr, uint64_t x) {
	*(ptr++) = uint8_t((x >> 0) & 0xFF);
	*(ptr++) = uint8_t((x >> 8) & 0xFF);
	*(ptr++) = uint8_t((x >> 16) & 0xFF);
	*(ptr++) = uint8_t((x >> 24) & 0xFF);
	*(ptr++) = uint8_t((x >> 32) & 0xFF);
	*(ptr++) = uint8_t((x >> 40) & 0xFF);
	*(ptr++) = uint8_t((x >> 48) & 0xFF);
	*(ptr++) = uint8_t((x >> 56) & 0xFF);
}
void WriteLE32(unsigned char* ptr, uint32_t x) {
	*(ptr++) = uint8_t((x >> 0) & 0xFF);
	*(ptr++) = uint8_t((x >> 8) & 0xFF);
	*(ptr++) = uint8_t((x >> 16) & 0xFF);
	*(ptr++) = uint8_t((x >> 24) & 0xFF);
}
uint32_t ReadLE32(unsigned char const* ptr) {
	return (uint32_t(ptr[0]) << 0)
	     | (uint32_t(ptr[1]) << 8)
	     | (uint32_t(ptr[2]) << 16)
	     | (uint32_t(ptr[3]) << 24)
	     ;
}

}
