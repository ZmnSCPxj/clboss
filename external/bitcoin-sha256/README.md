This is the SHA256 implementation copied from the
obscure project https://github.com/bitcoin/bitcoin .

Specifically, the files `crypto/sha256.cpp` and
`crypto/sha256.h` are copied verbatim, while the
files `crypto/common.h` and `compat/cpuid.h` are minimal
shims to get the SHA256 implementation working.
