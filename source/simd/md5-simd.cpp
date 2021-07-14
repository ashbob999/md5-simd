
#include "md5-simd.h"

#include <cstdio>
#include <stdexcept>

// intrinsic version

inline __m128i MD5_int::F(__m128i a, __m128i b, __m128i c, __m128i d)
{
	__m128i xor_c_d = _mm_xor_si128(c, d);
	__m128i and_b_xorc = _mm_and_si128(b, xor_c_d);
	__m128i res = _mm_xor_si128(d, and_b_xorc);

	return res;//_mm_xor_si128(d, _mm_and_si128(b, _mm_xor_si128(c, d)));
}

inline __m128i MD5_int::G(__m128i a, __m128i b, __m128i c, __m128i d)
{
	return _mm_xor_si128(c, _mm_and_si128(d, _mm_xor_si128(b, c)));
}

inline __m128i MD5_int::H(__m128i a, __m128i b, __m128i c, __m128i d)
{
	return _mm_xor_si128(b, _mm_xor_si128(c, d));
}

inline __m128i MD5_int::I(__m128i a, __m128i b, __m128i c, __m128i d)
{
	return _mm_xor_si128(c, _mm_or_si128(b, _mm_xor_si128(d, _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF))));
}

inline __m128i MD5_int::rotate_left(__m128i x, __m128i n)
{
	return _mm_or_si128(_mm_sllv_epi32(x, n),
		_mm_srlv_epi32(x, _mm_sub_epi32(_mm_set_epi32(32, 32, 32, 32), n)));
}

inline void MD5_int::FF(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac)
{
	__m128i tmp = _mm_add_epi32(a, _mm_add_epi32(F(a, b, c, d), _mm_add_epi32(x, ac)));
	//a = rotate_left(tmp, s);
	a = _mm_add_epi32(b, rotate_left(tmp, s));
}

inline void MD5_int::GG(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac)
{
	__m128i tmp = _mm_add_epi32(a, _mm_add_epi32(G(a, b, c, d), _mm_add_epi32(x, ac)));
	a = _mm_add_epi32(b, rotate_left(tmp, s));
}

inline void MD5_int::HH(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac)
{
	__m128i tmp = _mm_add_epi32(a, _mm_add_epi32(H(a, b, c, d), _mm_add_epi32(x, ac)));
	a = _mm_add_epi32(b, rotate_left(tmp, s));
}

inline void MD5_int::II(__m128i& a, __m128i b, __m128i c, __m128i d, __m128i x, __m128i s, __m128i ac)
{
	__m128i tmp = _mm_add_epi32(a, _mm_add_epi32(I(a, b, c, d), _mm_add_epi32(x, ac)));
	a = _mm_add_epi32(b, rotate_left(tmp, s));
}

MD5_int::MD5_int()
{
	init();
}

MD5_int::~MD5_int()
{
	delete[] strings[0];
	delete[] strings[1];
	delete[] strings[2];
	delete[] strings[3];
}

void MD5_int::calculate(const std::string text[4])
{
	reset();

	// check length
	uint64_t index = text[0].length() / 64;
	uint64_t offset = text[0].length() % 64;
	if (offset >= 56)
	{
		index++;
	}

	for (int i = 1; i < 4; i++)
	{
		uint64_t tmp_index = (text[i].length() / 64) + (text[i].length() % 64 < 56 ? 0 : 1);
		if (index != tmp_index)
		{
			throw "Lengths must be similar lengths";
		}
	}

	uint64_t total_chars = index * 64 + 64;
	if (total_chars > string_size)
	{
		delete[] strings[0];
		delete[] strings[1];
		delete[] strings[2];
		delete[] strings[3];

		strings[0] = new unsigned char[total_chars];
		strings[1] = new unsigned char[total_chars];
		strings[2] = new unsigned char[total_chars];
		strings[3] = new unsigned char[total_chars];
	}

	//pair p0 = resize(text[0].c_str(), text[0].length());
	//pair p1 = resize(text[1].c_str(), text[1].length());
	//pair p2 = resize(text[2].c_str(), text[2].length());
	//pair p3 = resize(text[3].c_str(), text[3].length());
	resize(text[0].c_str(), text[0].length(), 0);
	resize(text[1].c_str(), text[1].length(), 1);
	resize(text[2].c_str(), text[2].length(), 2);
	resize(text[3].c_str(), text[3].length(), 3);

	//unsigned char* arr[4] = { p0.str, p1.str, p2.str, p3.str };

	update(strings, total_chars);
	finalize();

	//delete p0.str, p1.str, p2.str, p3.str;
}

void MD5_int::calculate(const char* text[4], uint64_t length[4])
{
	reset();

	// check length
	uint64_t index = length[0] / 64;
	uint64_t offset = length[0] % 64;
	if (offset >= 56)
	{
		index++;
	}

	for (int i = 1; i < 4; i++)
	{
		uint64_t tmp_index = (length[i] / 64) + (length[i] % 64 < 56 ? 0 : 1);
		if (index != tmp_index)
		{
			throw std::runtime_error("Lengths must be similar");
		}
	}

	uint64_t total_chars = index * 64 + 64;
	if (total_chars > string_size)
	{
		delete[] strings[0];
		delete[] strings[1];
		delete[] strings[2];
		delete[] strings[3];

		strings[0] = new unsigned char[total_chars];
		strings[1] = new unsigned char[total_chars];
		strings[2] = new unsigned char[total_chars];
		strings[3] = new unsigned char[total_chars];
	}

	//pair p0 = resize(text[0], length[0]);
	//pair p1 = resize(text[1], length[1]);
	//pair p2 = resize(text[2], length[2]);
	//pair p3 = resize(text[3], length[3]);
	resize(text[0], length[0], 0);
	resize(text[1], length[1], 1);
	resize(text[2], length[2], 2);
	resize(text[3], length[3], 3);

	//unsigned char* arr[4] = {, p1.str, p2.str, p3.str };

	update(strings, total_chars);
	finalize();

	//delete p0.str, p1.str, p2.str, p3.str;
}

void MD5_int::calculate(char* text[4], uint64_t length[4])
{
	reset();

	// check length
	uint64_t index = length[0] / 64;
	uint64_t offset = length[0] % 64;
	if (offset >= 56)
	{
		index++;
	}

	for (int i = 1; i < 4; i++)
	{
		uint64_t tmp_index = (length[i] / 64) + (length[i] % 64 < 56 ? 0 : 1);
		if (index != tmp_index)
		{
			throw std::runtime_error("Lengths must be similar");
		}
	}

	uint64_t total_chars = index * 64 + 64;
	if (total_chars > string_size)
	{
		delete[] strings[0];
		delete[] strings[1];
		delete[] strings[2];
		delete[] strings[3];

		strings[0] = new unsigned char[total_chars];
		strings[1] = new unsigned char[total_chars];
		strings[2] = new unsigned char[total_chars];
		strings[3] = new unsigned char[total_chars];
	}

	//pair p0 = resize(text[0], length[0]);
	//pair p1 = resize(text[1], length[1]);
	//pair p2 = resize(text[2], length[2]);
	//pair p3 = resize(text[3], length[3]);
	resize(text[0], length[0], 0);
	resize(text[1], length[1], 1);
	resize(text[2], length[2], 2);
	resize(text[3], length[3], 3);

	//unsigned char* arr[4] = {, p1.str, p2.str, p3.str };

	update(strings, total_chars);
	finalize();

	//delete p0.str, p1.str, p2.str, p3.str;
}

void MD5_int::reset()
{
	finalized = false;

	//count[0] = 0;
	//count[1] = 0;
	count = 0;//count = _mm_setzero_si128();

	// load magic initialization constants.
	//state[0] = 0x67452301;
	//state[1] = 0xefcdab89;
	//state[2] = 0x98badcfe;
	//state[3] = 0x10325476;
	//state = _mm_setr_epi32(0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476);
	state[0] = _mm_set1_epi32(0x67452301);
	state[1] = _mm_set1_epi32(0xefcdab89);
	state[2] = _mm_set1_epi32(0x98badcfe);
	state[3] = _mm_set1_epi32(0x10325476);

	/*buffer[0] = _mm_setzero_si128();
	buffer[1] = _mm_setzero_si128();
	buffer[2] = _mm_setzero_si128();
	buffer[3] = _mm_setzero_si128();*/
	for (int i = 0; i < 4; i++)
	{
		buffer[i][0] = _mm_setzero_si128();
		buffer[i][1] = _mm_setzero_si128();
		buffer[i][2] = _mm_setzero_si128();
		buffer[i][3] = _mm_setzero_si128();
	}

	//digest = _mm_setzero_si128();
	digest[0] = _mm_setzero_si128();
	digest[1] = _mm_setzero_si128();
	digest[2] = _mm_setzero_si128();
	digest[3] = _mm_setzero_si128();
}

inline void MD5_int::resize(const char* text, uint64_t length, int idx)
{
	// get size of padding
	uint64_t index = length % 64;
	uint64_t padLen = (index < 56) ? (56 - index) : (120 - index);

	uint64_t new_length = length + padLen;

	//pair p;
	//p.str = new unsigned char[new_length + 8];
	//p.length = new_length + 8;

	// copy original string
	memcpy(strings[idx], text, length);

	// add 1 bit
	strings[idx][length] = 0x80;

	// set padding to zero
	memset(&strings[idx][length + 1], 0, padLen);

	uint64_t l = length * 8;

	// add length
	//p.str[length + padLen + 0] = ((length * 8) >> 0) & 0xff;
	//p.str[length + padLen + 1] = ((length * 8) >> 8) & 0xff;
	//p.str[length + padLen + 2] = ((length * 8) >> 16) & 0xff;
	//p.str[length + padLen + 3] = ((length * 8) >> 24) & 0xff;
	memcpy(&strings[idx][new_length], &l, 4);
	memset(&strings[idx][new_length + 4], 0, 4); // right-most 32 bits are zero

	//return p;
}

void MD5_int::update(unsigned char* input[4], uint64_t length)
{
	// compute number of bytes mod 64
	//size_type index = count[0] / 8 % blocksize;
	//size_type index = (count & 0xffffffff) / 8 % blocksize; //size_type index = _mm_extract_epi32(count, 0) / 8 % blocksize;
	uint64_t index = count / 8 % blocksize;

	// Update number of bits
	/*if ((count[0] += (length << 3)) < (length << 3))
	{
		count[1]++;
	}
	count[1] += (length >> 29);*/
	count += length << 3; // length * 8

	// number of bytes we need to fill in buffer
	uint64_t firstpart = 64 - index;

	uint64_t i;

	// transform as many times as possible.
	if (length >= firstpart)
	{
		// fill buffer first, transform
		//memcpy(&buffer[index], input, firstpart);
		//uint8_t* data = (uint8_t*) buffer;
		//memcpy(&data0[index], input, firstpart);

		uint8_t* data0 = (uint8_t*) buffer[0];
		memcpy(&data0[index], input[0], firstpart);

		uint8_t* data1 = (uint8_t*) buffer[1];
		memcpy(&data1[index], input[1], firstpart);

		uint8_t* data2 = (uint8_t*) buffer[2];
		memcpy(&data2[index], input[2], firstpart);

		uint8_t* data3 = (uint8_t*) buffer[3];
		memcpy(&data3[index], input[3], firstpart);

		transform(buffer);

		// transform chunks of blocksize (64 bytes)
		for (i = firstpart; i + blocksize <= length; i += blocksize)
		{
			__m128i next[4][4];
			/*next[0] = _mm_loadu_si128((__m128i*)(input + i));
			next[1] = _mm_loadu_si128((__m128i*)(input + i + 16));
			next[2] = _mm_loadu_si128((__m128i*)(input + i + 32));
			next[3] = _mm_loadu_si128((__m128i*)(input + i + 48));*/

			next[0][0] = _mm_loadu_si128((__m128i*)(input[0] + i));
			next[0][1] = _mm_loadu_si128((__m128i*)(input[0] + i + 16));
			next[0][2] = _mm_loadu_si128((__m128i*)(input[0] + i + 32));
			next[0][3] = _mm_loadu_si128((__m128i*)(input[0] + i + 48));

			next[1][0] = _mm_loadu_si128((__m128i*)(input[1] + i));
			next[1][1] = _mm_loadu_si128((__m128i*)(input[1] + i + 16));
			next[1][2] = _mm_loadu_si128((__m128i*)(input[1] + i + 32));
			next[1][3] = _mm_loadu_si128((__m128i*)(input[1] + i + 48));

			next[2][0] = _mm_loadu_si128((__m128i*)(input[2] + i));
			next[2][1] = _mm_loadu_si128((__m128i*)(input[2] + i + 16));
			next[2][2] = _mm_loadu_si128((__m128i*)(input[2] + i + 32));
			next[2][3] = _mm_loadu_si128((__m128i*)(input[2] + i + 48));

			next[3][0] = _mm_loadu_si128((__m128i*)(input[3] + i));
			next[3][1] = _mm_loadu_si128((__m128i*)(input[3] + i + 16));
			next[3][2] = _mm_loadu_si128((__m128i*)(input[3] + i + 32));
			next[3][3] = _mm_loadu_si128((__m128i*)(input[3] + i + 48));

			//transform(&input[i]);
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

//void MD5_int::update(const char* input, uint64_t length)
//{
//	update((const unsigned char*) input, length);
//}

inline void MD5_int::finalize()
{
	static unsigned char padding[64] = {
	  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	if (!finalized)
	{
		// Save number of bits
		unsigned char bits[8];
		//__m128i bits = _mm_setzero_si128();

		//__m128i cnt = _mm_set_epi64x(0, count);
		//encode(bits, count, 8);
		//encode(bits, cnt, 8);

		// pad out to 56 mod 64.
		//size_type index = count[0] / 8 % 64;
		//uint64_t index = (count & 0xffffffff) / 8 % 64;
		//uint64_t padLen = (index < 56) ? (56 - index) : (120 - index);
		//update(padding, padLen);

		// Append length (before padding)
		//update(bits, 8);

		// Store state in digest
		//encode(digest, state, 16);
		//uint8_t* dgst = (uint8_t*) &digest[0];
		encode(digest, state, 16 * 4);

		__m128 r0f = _mm_castsi128_ps(digest[0]);
		__m128 r1f = _mm_castsi128_ps(digest[1]);
		__m128 r2f = _mm_castsi128_ps(digest[2]);
		__m128 r3f = _mm_castsi128_ps(digest[3]);

		// because: digest[0] = state[0][0] | state[1][0] | state[2][0] | state[3][0]
		//_MM_TRANSPOSE4_PS(_mm_castsi128_ps(digest[0]), _mm_castsi128_ps(digest[1]), _mm_castsi128_ps(digest[2]), _mm_castsi128_ps(digest[3]));
		_MM_TRANSPOSE4_PS(r0f, r1f, r2f, r3f);

		digest[0] = _mm_castps_si128(r0f);
		digest[1] = _mm_castps_si128(r1f);
		digest[2] = _mm_castps_si128(r2f);
		digest[3] = _mm_castps_si128(r3f);

		// Zeroize sensitive information.
		//memset(buffer, 0, sizeof buffer);
		//memset(count, 0, sizeof count);

		finalized = true;
	}
}

std::string MD5_int::hexdigest(int index) const
{
	if (!finalized)
	{
		return "";
	}

	char buf[33];

	alignas(16) uint8_t data[16];
	_mm_storeu_si128((__m128i*)data, digest[3 - index]);

	for (int i = 0; i < 16; i++)
	{
		sprintf(buf + i * 2, "%02x", data[i]);
	}
	buf[32] = 0;

	return std::string(buf);
}

void MD5_int::hexdigest(char* str, int index) const
{
	if (!finalized)
	{
		return;
	}

	alignas(16) uint8_t data[16];
	_mm_storeu_si128((__m128i*)data, digest[3 - index]);

	for (int i = 0; i < 16; i++)
	{
		str[i * 2] = hex_mapping[data[i] >> 4];
		str[i * 2 + 1] = hex_mapping[data[i] & 0xf];
	}
}

void MD5_int::init()
{
	finalized = false;

	//count[0] = 0;
	//count[1] = 0;
	count = 0;//count = _mm_setzero_si128();

	// load magic initialization constants.
	//state[0] = 0x67452301;
	//state[1] = 0xefcdab89;
	//state[2] = 0x98badcfe;
	//state[3] = 0x10325476;
	//state = _mm_setr_epi32(0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476);
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

	strings[0] = new unsigned char[string_size];
	strings[1] = new unsigned char[string_size];
	strings[2] = new unsigned char[string_size];
	strings[3] = new unsigned char[string_size];
}

void MD5_int::transform(const __m128i block[4][4])
{
	//uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];
	//__m128i a = _mm_set1_epi32(_mm_extract_epi32(state, 0));
	//__m128i b = _mm_set1_epi32(_mm_extract_epi32(state, 1));
	//__m128i c = _mm_set1_epi32(_mm_extract_epi32(state, 2));
	//__m128i d = _mm_set1_epi32(_mm_extract_epi32(state, 3));
	__m128i a = state[0];
	__m128i b = state[1];
	__m128i c = state[2];
	__m128i d = state[3];

	__m128i x[16];

	decode (x, block, blocksize);
	//decode (x[0], block[0], blocksize);
	//decode (x[1], block[1], blocksize);
	//decode (x[2], block[2], blocksize);
	//decode (x[3], block[3], blocksize);

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

	/*state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;*/
	//state = _mm_add_epi32(state, _mm_insert_epi32(_mm_setzero_si128(), _mm_extract_epi32(a, 0), 0));
	//state = _mm_add_epi32(state, _mm_insert_epi32(_mm_setzero_si128(), _mm_extract_epi32(b, 0), 1));
	//state = _mm_add_epi32(state, _mm_insert_epi32(_mm_setzero_si128(), _mm_extract_epi32(c, 0), 2));
	//state = _mm_add_epi32(state, _mm_insert_epi32(_mm_setzero_si128(), _mm_extract_epi32(d, 0), 3));
	state[0] = _mm_add_epi32(state[0], a);
	state[1] = _mm_add_epi32(state[1], b);
	state[2] = _mm_add_epi32(state[2], c);
	state[3] = _mm_add_epi32(state[3], d);
}

inline void MD5_int::decode(__m128i output[], const __m128i input[4][4], uint64_t len)
{
	// 64 x 8-bits => 16 x 32-bits

	/*for (unsigned int i = 0, j = 0; j < len; i++, j += 4)
	{
		output[i] = ((uint32_t) input[j]) | (((uint32_t) input[j + 1]) << 8) |
			(((uint32_t) input[j + 2]) << 16) | (((uint32_t) input[j + 3]) << 24);
	}*/
	/*output[0] = _mm_loadu_si128(&input[0]);
	output[1] = _mm_loadu_si128(&input[1]);
	output[2] = _mm_loadu_si128(&input[2]);
	output[3] = _mm_loadu_si128(&input[3]);*/

	/*output[0] = _mm_set1_epi32(_mm_extract_epi32(input[0], 0));
	output[1] = _mm_set1_epi32(_mm_extract_epi32(input[0], 1));
	output[2] = _mm_set1_epi32(_mm_extract_epi32(input[0], 2));
	output[3] = _mm_set1_epi32(_mm_extract_epi32(input[0], 3));

	output[4] = _mm_set1_epi32(_mm_extract_epi32(input[1], 0));
	output[5] = _mm_set1_epi32(_mm_extract_epi32(input[1], 1));
	output[6] = _mm_set1_epi32(_mm_extract_epi32(input[1], 2));
	output[7] = _mm_set1_epi32(_mm_extract_epi32(input[1], 3));

	output[8] = _mm_set1_epi32(_mm_extract_epi32(input[2], 0));
	output[9] = _mm_set1_epi32(_mm_extract_epi32(input[2], 1));
	output[10] = _mm_set1_epi32(_mm_extract_epi32(input[2], 2));
	output[11] = _mm_set1_epi32(_mm_extract_epi32(input[2], 3));

	output[12] = _mm_set1_epi32(_mm_extract_epi32(input[3], 0));
	output[13] = _mm_set1_epi32(_mm_extract_epi32(input[3], 1));
	output[14] = _mm_set1_epi32(_mm_extract_epi32(input[3], 2));
	output[15] = _mm_set1_epi32(_mm_extract_epi32(input[3], 3));*/

	//output[0] = _mm_set_epi32();

	for (int i = 0, y = 0; i < 16; i += 4, y++)
	{
		output[i] = _mm_set_epi32(_mm_extract_epi32(input[0][y], 0), _mm_extract_epi32(input[1][y], 0), _mm_extract_epi32(input[2][y], 0), _mm_extract_epi32(input[3][y], 0));
		output[i + 1] = _mm_set_epi32(_mm_extract_epi32(input[0][y], 1), _mm_extract_epi32(input[1][y], 1), _mm_extract_epi32(input[2][y], 1), _mm_extract_epi32(input[3][y], 1));
		output[i + 2] = _mm_set_epi32(_mm_extract_epi32(input[0][y], 2), _mm_extract_epi32(input[1][y], 2), _mm_extract_epi32(input[2][y], 2), _mm_extract_epi32(input[3][y], 2));
		output[i + 3] = _mm_set_epi32(_mm_extract_epi32(input[0][y], 3), _mm_extract_epi32(input[1][y], 3), _mm_extract_epi32(input[2][y], 3), _mm_extract_epi32(input[3][y], 3));
	}
}

inline void MD5_int::encode(__m128i* output, const __m128i* input, uint64_t len)
{
	/*for (size_type i = 0, j = 0; j < len; i++, j += 4)
	{
		output[j] = input[i] & 0xff;
		output[j + 1] = (input[i] >> 8) & 0xff;
		output[j + 2] = (input[i] >> 16) & 0xff;
		output[j + 3] = (input[i] >> 24) & 0xff;
	}*/
	//memcpy(output, input, len);

	memcpy(output, input, len);
}

//std::string md5_int(const std::string str, int index)
//{
//	MD5_int md5 = MD5_int();
//
//	md5.calculate(str);
//
//	return md5.hexdigest(index);
//}

//void md5_int(const char* input, int input_length, char* output, int index)
//{
//	MD5_int md5 = MD5_int();
//	md5.calculate(input, input_length);
//	md5.hexdigest(output, index);
//}
