#ifndef MD5_SIMD_H
#define MD5_SIMD_H

// intrinsic version

#include <string>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <immintrin.h>

class MD5_SIMD
{
	static constexpr int HASH_COUNT = 4;
	static constexpr int BLOCK_SIZE = 64;
public:
	MD5_SIMD();
	MD5_SIMD(uint64_t buffer_size);

	// stop the md5 class being copied/moved
	MD5_SIMD(MD5_SIMD&) = delete;
	MD5_SIMD(const MD5_SIMD&) = delete;
	MD5_SIMD(MD5_SIMD&&) = delete;
	MD5_SIMD(const MD5_SIMD&&) = delete;

	~MD5_SIMD();

	// calculate methods for N hashes
	template<int N>
	void calculate(std::string text[N])
	{
		static_assert(N <= HASH_COUNT, "N must be <= the max number of hashes.");

		std::string texts[HASH_COUNT];
		for (int i = 0; i < N; i++)
		{
			texts[i] = text[i];
		}
		for (int rem = N; rem < HASH_COUNT; rem++)
		{
			texts[rem] = text[0];
		}

		calculate<HASH_COUNT>(texts);
	}

	// calculate methods for N hashes
	template<int N>
	void calculate(char* text[N], uint64_t length[N])
	{
		static_assert(N <= HASH_COUNT, "N must be <= the max number of hashes.");

		char* texts[HASH_COUNT];
		uint64_t lengths[HASH_COUNT];

		for (int i = 0; i < N; i++)
		{
			texts[i] = text[i];
			lengths[i] = length[i];
		}
		for (int rem = N; rem < HASH_COUNT; rem++)
		{
			texts[rem] = text[0];
			lengths[rem] = length[0];
		}

		calculate<HASH_COUNT>(texts, lengths);
	}

	// calculate methods for N hashes
	template<int N>
	void calculate(const char* text[N], uint64_t length[N])
	{
		static_assert(N <= HASH_COUNT, "N must be <= the max number of hashes.");

		const char* texts[HASH_COUNT];
		uint64_t lengths[HASH_COUNT];

		for (int i = 0; i < N; i++)
		{
			texts[i] = text[i];
			lengths[i] = length[i];
		}
		for (int rem = N; rem < HASH_COUNT; rem++)
		{
			texts[rem] = text[0];
			lengths[rem] = length[0];
		}

		calculate<HASH_COUNT>(texts, lengths);
	}

	// resets the md5 data and state
	void reset();

	// gets the hexdigest for the given hash index
	std::string hexdigest(int index) const;
	void hexdigest(char* str, int index) const;

private:
	void init();

	void pad_input(const char* text, uint64_t length, int idx);

	void update(unsigned char* buf[HASH_COUNT], uint64_t length);

	// stores 64 * 8-bits( block[hash_index][chunk_index] )
	void transform(const __m128i block[HASH_COUNT][4]);
	void finalize();

	static inline void decode(__m128i output[16], const __m128i input[HASH_COUNT][4], uint64_t len);
	static inline void encode(__m128i* output, const __m128i* input, uint64_t len);

	bool finalized;

	// bytes that didn't fit in last 64 byte chunk
	__m128i buffer[HASH_COUNT][(8 * BLOCK_SIZE) / 128];

	// counter for number of bits
	uint64_t count;

	// state per md5 is stored vertically (split into 32 bit chunks) (all a states are in state[0]) (a0 = state[0][0..31], b0 = state[1][0..31] and so on)
	__m128i state[HASH_COUNT];

	// stores the result, as 32 x 4 bits (or 16 uint8_t's) (hash 0 = digest[0])
	__m128i digest[HASH_COUNT];

	// buffers to store the padded inputs
	unsigned char* input_buffers[HASH_COUNT];
	uint64_t input_buffer_size = 64;

	static constexpr char HEX_MAPPING[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

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

	// per-round shift amounts
	static constexpr uint32_t r[] = {
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
	};

	// Use binary integer part of the sines of integers (Radians) as constants:
	static constexpr uint32_t k[] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
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
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};

	__m128i rv[sizeof(r) / sizeof(*r)]; // optimized for SSE
	__m128i kv[sizeof(k) / sizeof(*k)]; // optimized for SSE
};

// calculate methods for 4 hashes
template<>
inline void MD5_SIMD::calculate<MD5_SIMD::HASH_COUNT>(std::string text[MD5_SIMD::HASH_COUNT])
{
	// reset data
	reset();

	// get number of 64-char chunks the input will make
	uint64_t index = (text[0].length() / 64) + (text[0].length() % 64 < 56 ? 0 : 1);

	// check the other inputs to make sure they use the same number of 64-char chunks
	for (int i = 1; i < 4; i++)
	{
		uint64_t tmp_index = (text[i].length() / 64) + (text[i].length() % 64 < 56 ? 0 : 1);
		if (index != tmp_index)
		{
			throw std::runtime_error("Lengths must be similar");
		}
	}

	// calculate how many chars the inputs will each require (they should all require the same)
	uint64_t total_chars = index * 64 + 64;

	// expand the input buffers if they are not big enough
	if (total_chars > input_buffer_size)
	{
		delete[] input_buffers[0];
		delete[] input_buffers[1];
		delete[] input_buffers[2];
		delete[] input_buffers[3];

		input_buffers[0] = new unsigned char[total_chars];
		input_buffers[1] = new unsigned char[total_chars];
		input_buffers[2] = new unsigned char[total_chars];
		input_buffers[3] = new unsigned char[total_chars];
	}

	// pad each input
	pad_input(text[0].c_str(), text[0].length(), 0);
	pad_input(text[1].c_str(), text[1].length(), 1);
	pad_input(text[2].c_str(), text[2].length(), 2);
	pad_input(text[3].c_str(), text[3].length(), 3);

	// update the state
	update(input_buffers, total_chars);

	// finish the md5 calculation
	finalize();
}

// calculate methods for 4 hashes
template<>
inline void MD5_SIMD::calculate<MD5_SIMD::HASH_COUNT>(char* text[MD5_SIMD::HASH_COUNT], uint64_t length[MD5_SIMD::HASH_COUNT])
{
	// reset data
	reset();

	// get number of 64-char chunks the input will make
	uint64_t index = (length[0] / 64) + (length[0] % 64 < 56 ? 0 : 1);

	// check the other inputs to make sure they use the same number of 64-char chunks
	for (int i = 1; i < 4; i++)
	{
		uint64_t tmp_index = (length[i] / 64) + (length[i] % 64 < 56 ? 0 : 1);
		if (index != tmp_index)
		{
			throw std::runtime_error("Lengths must be similar");
		}
	}

	// calculate how many chars the inputs will each require (they should all require the same)
	uint64_t total_chars = index * 64 + 64;

	// expand the input buffers if they are not big enough
	if (total_chars > input_buffer_size)
	{
		delete[] input_buffers[0];
		delete[] input_buffers[1];
		delete[] input_buffers[2];
		delete[] input_buffers[3];

		input_buffers[0] = new unsigned char[total_chars];
		input_buffers[1] = new unsigned char[total_chars];
		input_buffers[2] = new unsigned char[total_chars];
		input_buffers[3] = new unsigned char[total_chars];
	}

	// pad each input
	pad_input(text[0], length[0], 0);
	pad_input(text[1], length[1], 1);
	pad_input(text[2], length[2], 2);
	pad_input(text[3], length[3], 3);

	// update the state
	update(input_buffers, total_chars);

	// finish the md5 calculation
	finalize();
}

// calculate methods for 4 hashes
template<>
inline void MD5_SIMD::calculate<MD5_SIMD::HASH_COUNT>(const char* text[MD5_SIMD::HASH_COUNT], uint64_t length[MD5_SIMD::HASH_COUNT])
{
	// reset data
	reset();

	// get number of 64-char chunks the input will make
	uint64_t index = (length[0] / 64) + (length[0] % 64 < 56 ? 0 : 1);

	// check the other inputs to make sure they use the same number of 64-char chunks
	for (int i = 1; i < 4; i++)
	{
		uint64_t tmp_index = (length[i] / 64) + (length[i] % 64 < 56 ? 0 : 1);
		if (index != tmp_index)
		{
			throw std::runtime_error("Lengths must be similar");
		}
	}

	// calculate how many chars the inputs will each require (they should all require the same)
	uint64_t total_chars = index * 64 + 64;

	// expand the input buffers if they are not big enough
	if (total_chars > input_buffer_size)
	{
		delete[] input_buffers[0];
		delete[] input_buffers[1];
		delete[] input_buffers[2];
		delete[] input_buffers[3];

		input_buffers[0] = new unsigned char[total_chars];
		input_buffers[1] = new unsigned char[total_chars];
		input_buffers[2] = new unsigned char[total_chars];
		input_buffers[3] = new unsigned char[total_chars];
	}

	// pad each input
	pad_input(text[0], length[0], 0);
	pad_input(text[1], length[1], 1);
	pad_input(text[2], length[2], 2);
	pad_input(text[3], length[3], 3);

	// update the state
	update(input_buffers, total_chars);

	// finish the md5 calculation
	finalize();
}

#endif
