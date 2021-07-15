
#include "md5-simd.h"

inline __m128i MD5_SIMD::F(__m128i a, __m128i b, __m128i c, __m128i d)
{
	return _mm_xor_si128(d, _mm_and_si128(b, _mm_xor_si128(c, d)));
}

inline __m128i MD5_SIMD::G(__m128i a, __m128i b, __m128i c, __m128i d)
{
	return _mm_xor_si128(c, _mm_and_si128(d, _mm_xor_si128(b, c)));
}

inline __m128i MD5_SIMD::H(__m128i a, __m128i b, __m128i c, __m128i d)
{
	return _mm_xor_si128(b, _mm_xor_si128(c, d));
}

inline __m128i MD5_SIMD::I(__m128i a, __m128i b, __m128i c, __m128i d)
{
	return _mm_xor_si128(c, _mm_or_si128(b, _mm_xor_si128(d, _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF))));
}

inline __m128i MD5_SIMD::rotate_left(__m128i x, __m128i n)
{
	return _mm_or_si128(_mm_sllv_epi32(x, n),
		_mm_srlv_epi32(x, _mm_sub_epi32(_mm_set_epi32(32, 32, 32, 32), n)));
}

inline void MD5_SIMD::FF(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac)
{
	__m128i tmp = _mm_add_epi32(a, _mm_add_epi32(F(a, b, c, d), _mm_add_epi32(x, ac)));
	a = _mm_add_epi32(b, rotate_left(tmp, s));
}

inline void MD5_SIMD::GG(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac)
{
	__m128i tmp = _mm_add_epi32(a, _mm_add_epi32(G(a, b, c, d), _mm_add_epi32(x, ac)));
	a = _mm_add_epi32(b, rotate_left(tmp, s));
}

inline void MD5_SIMD::HH(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac)
{
	__m128i tmp = _mm_add_epi32(a, _mm_add_epi32(H(a, b, c, d), _mm_add_epi32(x, ac)));
	a = _mm_add_epi32(b, rotate_left(tmp, s));
}

inline void MD5_SIMD::II(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac)
{
	__m128i tmp = _mm_add_epi32(a, _mm_add_epi32(I(a, b, c, d), _mm_add_epi32(x, ac)));
	a = _mm_add_epi32(b, rotate_left(tmp, s));
}

MD5_SIMD::MD5_SIMD()
{
	init();
}

MD5_SIMD::MD5_SIMD(uint64_t buffer_size)
{
	if (buffer_size == 0 || buffer_size % 64 != 0)
	{
		throw std::runtime_error("Buffer Size must be greater then zero, and a multiple of 64");
	}

	input_buffer_size = buffer_size;

	init();
}

MD5_SIMD::~MD5_SIMD()
{
	delete[] input_buffers[0];
	delete[] input_buffers[1];
	delete[] input_buffers[2];
	delete[] input_buffers[3];
}

void MD5_SIMD::init()
{
	finalized = false;

	count = 0;

	// load magic initialization constants.
	state[0] = _mm_set1_epi32(0x67452301);
	state[1] = _mm_set1_epi32(0xefcdab89);
	state[2] = _mm_set1_epi32(0x98badcfe);
	state[3] = _mm_set1_epi32(0x10325476);

	int i;

	for (i = 0; i < (sizeof(rv) / sizeof(*rv)); i++)
	{
		rv[i] = _mm_set_epi32(r[i], r[i], r[i], r[i]);
	}

	for (i = 0; i < (sizeof(kv) / sizeof(*kv)); i++)
	{
		kv[i] = _mm_set_epi32(k[i], k[i], k[i], k[i]);
	}

	input_buffers[0] = new unsigned char[input_buffer_size];
	input_buffers[1] = new unsigned char[input_buffer_size];
	input_buffers[2] = new unsigned char[input_buffer_size];
	input_buffers[3] = new unsigned char[input_buffer_size];
}

void MD5_SIMD::reset()
{
	finalized = false;

	// reset count
	count = 0;

	// reset state
	state[0] = _mm_set1_epi32(0x67452301);
	state[1] = _mm_set1_epi32(0xefcdab89);
	state[2] = _mm_set1_epi32(0x98badcfe);
	state[3] = _mm_set1_epi32(0x10325476);

	// clear buffer
	for (int i = 0; i < 4; i++)
	{
		buffer[i][0] = _mm_setzero_si128();
		buffer[i][1] = _mm_setzero_si128();
		buffer[i][2] = _mm_setzero_si128();
		buffer[i][3] = _mm_setzero_si128();
	}

	// clear digest
	digest[0] = _mm_setzero_si128();
	digest[1] = _mm_setzero_si128();
	digest[2] = _mm_setzero_si128();
	digest[3] = _mm_setzero_si128();
}

void MD5_SIMD::pad_input(const char* text, uint64_t length, int idx)
{
	// get size of padding
	uint64_t index = length % 64;
	uint64_t padLen = (index < 56) ? (56 - index) : (120 - index);

	// calculate length + padding
	uint64_t new_length = length + padLen;

	// copy original string
	memcpy(input_buffers[idx], text, length);

	// add 1 bit
	input_buffers[idx][length] = 0x80;

	// set padding to zero
	memset(&input_buffers[idx][length + 1], 0, padLen);

	// get length of input in bits
	uint64_t bit_length = length * 8;

	// add length to end of input
	memcpy(&input_buffers[idx][new_length], &bit_length, 8);
}

void MD5_SIMD::update(unsigned char* input[HASH_COUNT], uint64_t length)
{
	// compute number of bytes mod 64
	uint64_t index = count / 8 % BLOCK_SIZE;

	// Update number of bits
	count += length << 3; // length * 8

	// number of bytes we need to fill in buffer
	uint64_t firstpart = BLOCK_SIZE - index;

	uint64_t i;

	// transform as many times as possible.
	if (length >= firstpart)
	{
		// fill buffer first, transform
		memcpy(buffer[0] + index, input[0], firstpart);
		memcpy(buffer[1] + index, input[1], firstpart);
		memcpy(buffer[2] + index, input[2], firstpart);
		memcpy(buffer[3] + index, input[3], firstpart);

		transform(buffer);

		// transform chunks of blocksize (64 bytes)
		for (i = firstpart; i + BLOCK_SIZE <= length; i += BLOCK_SIZE)
		{
			__m128i next[HASH_COUNT][4];

			for (int hash_index = 0; hash_index < HASH_COUNT; hash_index++)
			{
				next[hash_index][0] = _mm_loadu_si128((__m128i*)(input[0] + i));
				next[hash_index][1] = _mm_loadu_si128((__m128i*)(input[0] + i + 16));
				next[hash_index][2] = _mm_loadu_si128((__m128i*)(input[0] + i + 32));
				next[hash_index][3] = _mm_loadu_si128((__m128i*)(input[0] + i + 48));
			}

			transform(next);
		}

		index = 0;
	}
	else
	{
		i = 0;
	}

	int chars_left = length - i;

	// buffer remaining input
	//memcpy(&buffer[index], &input[i], chars_left);

	uint8_t* data = (uint8_t*) &buffer;
	memcpy(&data[index], &input[i], chars_left);
}

void MD5_SIMD::transform(const __m128i block[HASH_COUNT][4])
{
	__m128i a = state[0];
	__m128i b = state[1];
	__m128i c = state[2];
	__m128i d = state[3];

	__m128i x[16];

	decode (x, block, BLOCK_SIZE);

	/* Round 1 */
	FF (a, b, c, d, x[0], rv[0], kv[0]); /* 1 */
	FF (d, a, b, c, x[1], rv[1], kv[1]); /* 2 */
	FF (c, d, a, b, x[2], rv[2], kv[2]); /* 3 */
	FF (b, c, d, a, x[3], rv[3], kv[3]); /* 4 */
	FF (a, b, c, d, x[4], rv[4], kv[4]); /* 5 */
	FF (d, a, b, c, x[5], rv[5], kv[5]); /* 6 */
	FF (c, d, a, b, x[6], rv[6], kv[6]); /* 7 */
	FF (b, c, d, a, x[7], rv[7], kv[7]); /* 8 */
	FF (a, b, c, d, x[8], rv[8], kv[8]); /* 9 */
	FF (d, a, b, c, x[9], rv[9], kv[9]); /* 10 */
	FF (c, d, a, b, x[10], rv[10], kv[10]); /* 11 */
	FF (b, c, d, a, x[11], rv[11], kv[11]); /* 12 */
	FF (a, b, c, d, x[12], rv[12], kv[12]); /* 13 */
	FF (d, a, b, c, x[13], rv[13], kv[13]); /* 14 */
	FF (c, d, a, b, x[14], rv[14], kv[14]); /* 15 */
	FF (b, c, d, a, x[15], rv[15], kv[15]); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[1], rv[16], kv[16]); /* 17 */
	GG (d, a, b, c, x[6], rv[17], kv[17]); /* 18 */
	GG (c, d, a, b, x[11], rv[18], kv[18]); /* 19 */
	GG (b, c, d, a, x[0], rv[19], kv[19]); /* 20 */
	GG (a, b, c, d, x[5], rv[20], kv[20]); /* 21 */
	GG (d, a, b, c, x[10], rv[21], kv[21]); /* 22 */
	GG (c, d, a, b, x[15], rv[22], kv[22]); /* 23 */
	GG (b, c, d, a, x[4], rv[23], kv[23]); /* 24 */
	GG (a, b, c, d, x[9], rv[24], kv[24]); /* 25 */
	GG (d, a, b, c, x[14], rv[25], kv[25]); /* 26 */
	GG (c, d, a, b, x[3], rv[26], kv[26]); /* 27 */
	GG (b, c, d, a, x[8], rv[27], kv[27]); /* 28 */
	GG (a, b, c, d, x[13], rv[28], kv[28]); /* 29 */
	GG (d, a, b, c, x[2], rv[29], kv[29]); /* 30 */
	GG (c, d, a, b, x[7], rv[30], kv[30]); /* 31 */
	GG (b, c, d, a, x[12], rv[31], kv[31]); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[5], rv[32], kv[32]); /* 33 */
	HH (d, a, b, c, x[8], rv[33], kv[33]); /* 34 */
	HH (c, d, a, b, x[11], rv[34], kv[34]); /* 35 */
	HH (b, c, d, a, x[14], rv[35], kv[35]); /* 36 */
	HH (a, b, c, d, x[1], rv[36], kv[36]); /* 37 */
	HH (d, a, b, c, x[4], rv[37], kv[37]); /* 38 */
	HH (c, d, a, b, x[7], rv[38], kv[38]); /* 39 */
	HH (b, c, d, a, x[10], rv[39], kv[39]); /* 40 */
	HH (a, b, c, d, x[13], rv[40], kv[40]); /* 41 */
	HH (d, a, b, c, x[0], rv[41], kv[41]); /* 42 */
	HH (c, d, a, b, x[3], rv[42], kv[42]); /* 43 */
	HH (b, c, d, a, x[6], rv[43], kv[43]); /* 44 */
	HH (a, b, c, d, x[9], rv[44], kv[44]); /* 45 */
	HH (d, a, b, c, x[12], rv[45], kv[45]); /* 46 */
	HH (c, d, a, b, x[15], rv[46], kv[46]); /* 47 */
	HH (b, c, d, a, x[2], rv[47], kv[47]); /* 48 */

	/* Round 4 */
	II (a, b, c, d, x[0], rv[48], kv[48]); /* 49 */
	II (d, a, b, c, x[7], rv[49], kv[49]); /* 50 */
	II (c, d, a, b, x[14], rv[50], kv[50]); /* 51 */
	II (b, c, d, a, x[5], rv[51], kv[51]); /* 52 */
	II (a, b, c, d, x[12], rv[52], kv[52]); /* 53 */
	II (d, a, b, c, x[3], rv[53], kv[53]); /* 54 */
	II (c, d, a, b, x[10], rv[54], kv[54]); /* 55 */
	II (b, c, d, a, x[1], rv[55], kv[55]); /* 56 */
	II (a, b, c, d, x[8], rv[56], kv[56]); /* 57 */
	II (d, a, b, c, x[15], rv[57], kv[57]); /* 58 */
	II (c, d, a, b, x[6], rv[58], kv[58]); /* 59 */
	II (b, c, d, a, x[13], rv[59], kv[59]); /* 60 */
	II (a, b, c, d, x[4], rv[60], kv[60]); /* 61 */
	II (d, a, b, c, x[11], rv[61], kv[61]); /* 62 */
	II (c, d, a, b, x[2], rv[62], kv[62]); /* 63 */
	II (b, c, d, a, x[9], rv[63], kv[63]); /* 64 */

	// update state
	state[0] = _mm_add_epi32(state[0], a);
	state[1] = _mm_add_epi32(state[1], b);
	state[2] = _mm_add_epi32(state[2], c);
	state[3] = _mm_add_epi32(state[3], d);
}

void MD5_SIMD::finalize()
{
	static unsigned char padding[64] = {
	  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	if (!finalized)
	{
		// Store state in digest
		encode(digest, state, 16 * 4);

		// transpose requires floats
		__m128 r0f = _mm_castsi128_ps(digest[0]);
		__m128 r1f = _mm_castsi128_ps(digest[1]);
		__m128 r2f = _mm_castsi128_ps(digest[2]);
		__m128 r3f = _mm_castsi128_ps(digest[3]);

		// because: digest[0] = state[0][0] | state[1][0] | state[2][0] | state[3][0]
		_MM_TRANSPOSE4_PS(r0f, r1f, r2f, r3f);

		digest[0] = _mm_castps_si128(r0f);
		digest[1] = _mm_castps_si128(r1f);
		digest[2] = _mm_castps_si128(r2f);
		digest[3] = _mm_castps_si128(r3f);

		finalized = true;
	}
}

inline void MD5_SIMD::decode(__m128i output[16], const __m128i input[HASH_COUNT][4], uint64_t len)
{
	// 64 x 8-bits => 16 x 32-bits for each hash

	int* data = (int*) &input[0];

	__m128i offset = _mm_setr_epi32(0, 16, 32, 48);

	for (int i = 0, y = 0; i < 16; i += 4, y++)
	{
		output[i] = _mm_i32gather_epi32(data + i + 0, offset, 4);
		output[i + 1] = _mm_i32gather_epi32(data + i + 1, offset, 4);
		output[i + 2] = _mm_i32gather_epi32(data + i + 2, offset, 4);
		output[i + 3] = _mm_i32gather_epi32(data + i + 3, offset, 4);
	}
}

inline void MD5_SIMD::encode(__m128i* output, const __m128i* input, uint64_t len)
{
	memcpy(output, input, len);
}

std::string MD5_SIMD::hexdigest(int index) const
{
	if (!finalized)
	{
		return "";
	}

	char buf[33];

	uint8_t data[16];
	_mm_storeu_si128((__m128i*)data, digest[index]);

	for (int i = 0; i < 16; i++)
	{
		buf[i * 2] = HEX_MAPPING[data[i] >> 4];
		buf[i * 2 + 1] = HEX_MAPPING[data[i] & 0xf];
	}
	buf[32] = 0;

	return std::string(buf);
}

void MD5_SIMD::hexdigest(char* str, int index) const
{
	if (!finalized)
	{
		return;
	}

	uint8_t data[16];
	_mm_storeu_si128((__m128i*)data, digest[index]);

	for (int i = 0; i < 16; i++)
	{
		str[i * 2] = HEX_MAPPING[data[i] >> 4];
		str[i * 2 + 1] = HEX_MAPPING[data[i] & 0xf];
	}
}
