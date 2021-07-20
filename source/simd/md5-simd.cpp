
#include "md5-simd.h"

inline __reg MD5_SIMD::F(__reg a, __reg b, __reg c, __reg d)
{
	return _xor_si(d, _and_si(b, _xor_si(c, d)));
}

inline __reg MD5_SIMD::G(__reg a, __reg b, __reg c, __reg d)
{
	return _xor_si(c, _and_si(d, _xor_si(b, c)));
}

inline __reg MD5_SIMD::H(__reg a, __reg b, __reg c, __reg d)
{
	return _xor_si(b, _xor_si(c, d));
}

inline __reg MD5_SIMD::I(__reg a, __reg b, __reg c, __reg d)
{
	return _xor_si(c, _or_si(b, _xor_si(d, _set1_epi32(0xFFFFFFFF))));
}

inline __reg MD5_SIMD::rotate_left(__reg x, __reg n)
{
	return _or_si(_sllv_epi32(x, n),
		_srlv_epi32(x, _sub_epi32(_set1_epi32(32), n)));
}

inline void MD5_SIMD::FF(__reg& a, __reg b, __reg c, __reg d, __reg x, __reg s, __reg ac)
{
	__reg tmp = _add_epi32(a, _add_epi32(F(a, b, c, d), _add_epi32(x, ac)));
	a = _add_epi32(b, rotate_left(tmp, s));
}

inline void MD5_SIMD::GG(__reg& a, __reg b, __reg c, __reg d, __reg x, __reg s, __reg ac)
{
	__reg tmp = _add_epi32(a, _add_epi32(G(a, b, c, d), _add_epi32(x, ac)));
	a = _add_epi32(b, rotate_left(tmp, s));
}

inline void MD5_SIMD::HH(__reg& a, __reg b, __reg c, __reg d, __reg x, __reg s, __reg ac)
{
	__reg tmp = _add_epi32(a, _add_epi32(H(a, b, c, d), _add_epi32(x, ac)));
	a = _add_epi32(b, rotate_left(tmp, s));
}

inline void MD5_SIMD::II(__reg& a, __reg b, __reg c, __reg d, __reg x, __reg s, __reg ac)
{
	__reg tmp = _add_epi32(a, _add_epi32(I(a, b, c, d), _add_epi32(x, ac)));
	a = _add_epi32(b, rotate_left(tmp, s));
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
	for (int i = 0; i < HASH_COUNT; i++)
	{
		delete[] input_buffers[i];
	}
}

void MD5_SIMD::init()
{
	reset();

	int i;

	for (i = 0; i < (sizeof(rv) / sizeof(*rv)); i++)
	{
		rv[i] = _set1_epi32(r[i]);
	}

	for (i = 0; i < (sizeof(kv) / sizeof(*kv)); i++)
	{
		kv[i] = _set1_epi32(k[i]);
	}

	for (int hash_index = 0; hash_index < HASH_COUNT; hash_index++)
	{
		input_buffers[hash_index] = new unsigned char[input_buffer_size];
	}
}

void MD5_SIMD::reset()
{
	finalized = false;

	// reset count
	count = 0;

	// reset state
	state[0] = _set1_epi32(0x67452301);
	state[1] = _set1_epi32(0xefcdab89);
	state[2] = _set1_epi32(0x98badcfe);
	state[3] = _set1_epi32(0x10325476);

	// clear buffer
	for (int i = 0; i < HASH_COUNT; i++)
	{
		buffer[i][0] = _setzero_si128();
		buffer[i][1] = _setzero_si128();
		buffer[i][2] = _setzero_si128();
		buffer[i][3] = _setzero_si128();
	}

	// clear digest
	for (int i = 0; i < HASH_COUNT; i++)
	{
		digest[i] = _setzero_si();
	}
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
		for (int hash_index = 0; hash_index < HASH_COUNT; hash_index++)
		{
			memcpy(buffer[hash_index] + index, input[hash_index], firstpart);
		}

		transform(buffer);

		// transform chunks of blocksize (64 bytes)
		for (i = firstpart; i + BLOCK_SIZE <= length; i += BLOCK_SIZE)
		{
			__reg128 next[HASH_COUNT][4];

			for (int hash_index = 0; hash_index < HASH_COUNT; hash_index++)
			{
				next[hash_index][0] = _loadu_si128((__reg128*) (input[0] + i));
				next[hash_index][1] = _loadu_si128((__reg128*) (input[0] + i + 16));
				next[hash_index][2] = _loadu_si128((__reg128*) (input[0] + i + 32));
				next[hash_index][3] = _loadu_si128((__reg128*) (input[0] + i + 48));
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

void MD5_SIMD::transform(const __reg128 block[HASH_COUNT][4])
{
	__reg a = state[0];
	__reg b = state[1];
	__reg c = state[2];
	__reg d = state[3];

	__reg x[16];

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
	state[0] = _add_epi32(state[0], a);
	state[1] = _add_epi32(state[1], b);
	state[2] = _add_epi32(state[2], c);
	state[3] = _add_epi32(state[3], d);
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
		encode(digest, state, 16 * HASH_COUNT);

		// because: digest[0] = state[0][0] | state[1][0] | state[2][0] | state[3][0]
		transpose(digest);

		finalized = true;
	}
}

inline void MD5_SIMD::transpose(__reg digest[HASH_COUNT])
{
	//using namespace simd_functions;

	// casting is needed because transpose requires floats
#ifdef USE_256_BITS
	__reg_ps row0 = _castsi_ps(digest[0]);
	__reg_ps row1 = _castsi_ps(digest[1]);
	__reg_ps row2 = _castsi_ps(digest[2]);
	__reg_ps row3 = _castsi_ps(digest[3]);
	__reg_ps row4 = _castsi_ps(digest[4]);
	__reg_ps row5 = _castsi_ps(digest[5]);
	__reg_ps row6 = _castsi_ps(digest[6]);
	__reg_ps row7 = _castsi_ps(digest[7]);

	__reg_ps __t0, __t1, __t2, __t3, __t4, __t5, __t6, __t7;
	__reg_ps __tt0, __tt1, __tt2, __tt3, __tt4, __tt5, __tt6, __tt7;

	__t0 = _unpacklo_ps(row0, row1);
	__t1 = _unpackhi_ps(row0, row1);
	__t2 = _unpacklo_ps(row2, row3);
	__t3 = _unpackhi_ps(row2, row3);
	__t4 = _unpacklo_ps(row4, row5);
	__t5 = _unpackhi_ps(row4, row5);
	__t6 = _unpacklo_ps(row6, row7);
	__t7 = _unpackhi_ps(row6, row7);
	__tt0 = _shuffle_ps(__t0, __t2, _MM_SHUFFLE(1, 0, 1, 0));
	__tt1 = _shuffle_ps(__t0, __t2, _MM_SHUFFLE(3, 2, 3, 2));
	__tt2 = _shuffle_ps(__t1, __t3, _MM_SHUFFLE(1, 0, 1, 0));
	__tt3 = _shuffle_ps(__t1, __t3, _MM_SHUFFLE(3, 2, 3, 2));
	__tt4 = _shuffle_ps(__t4, __t6, _MM_SHUFFLE(1, 0, 1, 0));
	__tt5 = _shuffle_ps(__t4, __t6, _MM_SHUFFLE(3, 2, 3, 2));
	__tt6 = _shuffle_ps(__t5, __t7, _MM_SHUFFLE(1, 0, 1, 0));
	__tt7 = _shuffle_ps(__t5, __t7, _MM_SHUFFLE(3, 2, 3, 2));
	row0 = _permute2f128_ps(__tt0, __tt4, 0x20);
	row1 = _permute2f128_ps(__tt1, __tt5, 0x20);
	row2 = _permute2f128_ps(__tt2, __tt6, 0x20);
	row3 = _permute2f128_ps(__tt3, __tt7, 0x20);
	row4 = _permute2f128_ps(__tt0, __tt4, 0x31);
	row5 = _permute2f128_ps(__tt1, __tt5, 0x31);
	row6 = _permute2f128_ps(__tt2, __tt6, 0x31);
	row7 = _permute2f128_ps(__tt3, __tt7, 0x31);

	digest[0] = _castps_si(row0);
	digest[1] = _castps_si(row1);
	digest[2] = _castps_si(row2);
	digest[3] = _castps_si(row3);
	digest[4] = _castps_si(row4);
	digest[5] = _castps_si(row5);
	digest[6] = _castps_si(row6);
	digest[7] = _castps_si(row7);
#else
	__reg_ps r0f = _castsi_ps(digest[0]);
	__reg_ps r1f = _castsi_ps(digest[1]);
	__reg_ps r2f = _castsi_ps(digest[2]);
	__reg_ps r3f = _castsi_ps(digest[3]);

	//_MM_TRANSPOSE4_PS(r0f, r1f, r2f, r3f);

	__reg_ps tmp3, tmp2, tmp1, tmp0;
	tmp0 = _unpacklo_ps(r0f, r1f);
	tmp2 = _unpacklo_ps(r2f, r3f);
	tmp1 = _unpackhi_ps(r0f, r1f);
	tmp3 = _unpackhi_ps(r2f, r3f);
	r0f = _movelh_ps(tmp0, tmp2);
	r1f = _movehl_ps(tmp2, tmp0);
	r2f = _movelh_ps(tmp1, tmp3);
	r3f = _movehl_ps(tmp3, tmp1);

	digest[0] = _castps_si(r0f);
	digest[1] = _castps_si(r1f);
	digest[2] = _castps_si(r2f);
	digest[3] = _castps_si(r3f);
#endif
}

inline void MD5_SIMD::decode(__reg output[16], const __reg128 input[HASH_COUNT][4], uint64_t len)
{
	// 64 x 8-bits => 16 x 32-bits for each hash

	int* data = (int*) &input[0];

#ifdef USE_256_BITS
	__reg offset = _setr_epi32(0, 16, 32, 48, 64, 80, 96, 112);
#else
	__reg offset = _setr_epi32(0, 16, 32, 48);
#endif

	for (int i = 0, y = 0; i < 16; i += 4, y++)
	{
		output[i] = _i32gather_epi32(data + i + 0, offset, 4);
		output[i + 1] = _i32gather_epi32(data + i + 1, offset, 4);
		output[i + 2] = _i32gather_epi32(data + i + 2, offset, 4);
		output[i + 3] = _i32gather_epi32(data + i + 3, offset, 4);
	}
}

inline void MD5_SIMD::encode(__reg* output, const __reg* input, uint64_t len)
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

	__reg128* tmp_pointer = (__reg128*) &digest[index];

	_storeu_si128((__reg128*) data, *tmp_pointer);

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

	__reg128* tmp_pointer = (__reg128*) &digest[index];

	_storeu_si128((__reg128*) data, *tmp_pointer);

	for (int i = 0; i < 16; i++)
	{
		str[i * 2] = HEX_MAPPING[data[i] >> 4];
		str[i * 2 + 1] = HEX_MAPPING[data[i] & 0xf];
	}
}
