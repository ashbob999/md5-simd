/* MD5
 converted to C++ class by Frank Thilo (thilo@unix-ag.org)
 for bzflag (http://www.bzflag.org)

   based on:

   md5.h and md5.c
   reference implementation of RFC 1321

   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

This has been derived/modified for speed for Advent of Code puzzles.
plus code from: https://github.com/crossbowerbt/md5/blob/master/md5_sse.c
*/

#ifndef BZF_MD5_H
#define BZF_MD5_H

#include <cstring>
#include <iostream>
#include <cstdint>

constexpr char hex_mapping[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

// a small class for calculating MD5 hashes of strings or byte arrays
// it is not meant to be fast or secure
//
// usage: 1) feed it blocks of uchars with update()
//      2) finalize()
//      3) get hexdigest() string
//      or
//      MD5(std::string).hexdigest()
//
// assumes that char is 8 bit and int is 32 bit
class MD5
{
public:
	typedef unsigned int size_type; // must be 32bit

	MD5();
	//MD5() = delete;
	//MD5(const std::string& text);
	//MD5(const char* text, int length);

	MD5(MD5&) = delete;
	MD5(const MD5&) = delete;

	MD5(MD5&&) = delete;
	MD5(const MD5&&) = delete;

	void calculate(const std::string text);
	void calculate(const char* text, uint64_t length);
	void calculate(char* text, uint64_t length);
	void reset();

	void update(const unsigned char* buf, size_type length);
	void update(const char* buf, size_type length);
	MD5& finalize();
	std::string hexdigest() const;
	void hexdigest(char* str) const;

private:
	void init();
	enum { blocksize = 64 }; // VC6 won't eat a const static int here

	void transform(const uint8_t block[blocksize]);
	static void decode(uint32_t output[], const uint8_t input[], size_type len);
	static void encode(uint8_t output[], const uint32_t input[], size_type len);

	bool finalized;
	uint8_t buffer[blocksize]; // bytes that didn't fit in last 64 byte chunk
	uint32_t count[2];   // 64bit counter for number of bits (lo, hi)
	uint32_t state[4];   // digest so far
	uint8_t digest[16]; // the result

	// low level logic operations
	static inline uint32_t F(uint32_t x, uint32_t y, uint32_t z);
	static inline uint32_t G(uint32_t x, uint32_t y, uint32_t z);
	static inline uint32_t H(uint32_t x, uint32_t y, uint32_t z);
	static inline uint32_t I(uint32_t x, uint32_t y, uint32_t z);
	static inline uint32_t rotate_left(uint32_t x, int n);
	static inline void FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac);
	static inline void GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac);
	static inline void HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac);
	static inline void II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac);
};

std::string md5(const std::string str);
void md5(const char* input, int input_length, char* output);

#endif
