#ifndef MD5_SIMD_H
#define MD5_SIMD_H

// intrinsic version

#include <string>
#include <algorithm>
#include <immintrin.h>

#include "../original/md5-original.h"
//constexpr char hex_mapping[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

constexpr uint32_t r[] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
						   5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
						   4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
						   6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };


constexpr uint32_t k[] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
						   0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
						   0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
						   0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
						   0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
						   0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
						   0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
						   0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
						   0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
						   0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
						   0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
						   0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
						   0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
						   0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
						   0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
						   0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

class MD5_int
{
public:
	//typedef unsigned int size_type; // must be 32bit

	struct pairaaa
	{
		unsigned char* str;
		uint64_t length;
	};

	MD5_int();

	MD5_int(MD5_int&) = delete;
	MD5_int(const MD5_int&) = delete;

	MD5_int(MD5_int&&) = delete;
	MD5_int(const MD5_int&&) = delete;

	~MD5_int();

	void calculate(const std::string text[4]);
	void calculate(const char* text[4], uint64_t length[4]);
	void calculate(char* text[4], uint64_t length[4]);
	void reset();

	inline void resize(const char* text, uint64_t length, int idx);
	void update(unsigned char* buf[4], uint64_t length);
	//void update(const char* buf, uint64_t length);
	inline void finalize();
	std::string hexdigest(int index) const;
	void hexdigest(char* str, int index) const;

private:
	void init();
	enum { blocksize = 64 }; // VC6 won't eat a const static int here

	void transform(const __m128i block[4][4]);
	static inline void decode(__m128i output[], const __m128i input[4][4], uint64_t len);
	static inline void encode(__m128i* output, const __m128i* input, uint64_t len);

	bool finalized;
	//uint8_t buffer[blocksize]; // bytes that didn't fit in last 64 byte chunk
	__m128i buffer[4][(8 * blocksize) / 128];//__m128i buffer[(8 * blocksize) / 128];
	//uint32_t count[2];   // 64bit counter for number of bits (lo, hi)
	uint64_t count;//__m128i count;
	//uint32_t state[4];   // digest so far
	__m128i state[4]; // __m128i state;
	//uint8_t digest[16]; // the result
	__m128i digest[4]; //__m128i digest;

	__m128i rv[sizeof(r) / sizeof(*r)]; // optimized for SSE
	__m128i kv[sizeof(k) / sizeof(*k)]; // optimized for SSE

	unsigned char* strings[4];
	uint64_t string_size = 64;

	// low level logic operations
	static inline __m128i F(__m128i a, __m128i b, __m128i c, __m128i d);
	static inline __m128i G(__m128i a, __m128i b, __m128i c, __m128i d);
	static inline __m128i H(__m128i a, __m128i b, __m128i c, __m128i d);
	static inline __m128i I(__m128i a, __m128i b, __m128i c, __m128i d);
	static inline __m128i rotate_left(__m128i x, __m128i n);
	static inline void FF(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac);
	static inline void GG(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac);
	static inline void HH(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac);
	static inline void II(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac);
};

//std::string md5_int(const std::string str, int index);
//void md5_int(const char* input, int input_length, char* output, int index);

#endif
