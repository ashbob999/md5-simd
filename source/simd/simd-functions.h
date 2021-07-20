
#ifndef SIMD_FUNCTIONS_H
#define SIMD_FUNCTIONS_H

#include <immintrin.h>

#ifdef USE_256_BITS

#define __reg __m256i
#define __reg_ps __m256

#define _add_epi32 _mm256_add_epi32
#define _sub_epi32 _mm256_sub_epi32

#define _and_si _mm256_and_si256
#define _or_si _mm256_or_si256
#define _xor_si _mm256_xor_si256

#define _sllv_epi32 _mm256_sllv_epi32
#define _srlv_epi32 _mm256_srlv_epi32
#define _slli_epi32 _mm256_slli_epi32
#define _srli_epi32 _mm256_srli_epi32

#define _set1_epi32 _mm256_set1_epi32
#define _setr_epi32 _mm256_setr_epi32
#define _setzero_si _mm256_setzero_si256
#define _i32gather_epi32 _mm256_i32gather_epi32

#define _cast_si128 _mm256_castsi256_si128
#define _castsi_ps _mm256_castsi256_ps
#define _castps_si _mm256_castps_si256

#define _unpackhi_ps _mm256_unpackhi_ps
#define _unpacklo_ps _mm256_unpacklo_ps
#define _shuffle_ps _mm256_shuffle_ps
#define _permute2f128_ps _mm256_permute2f128_ps

#else

#define __reg __m128i
#define __reg_ps __m128

#define _add_epi32 _mm_add_epi32
#define _sub_epi32 _mm_sub_epi32

#define _and_si _mm_and_si128
#define _or_si _mm_or_si128
#define _xor_si _mm_xor_si128

#define _sllv_epi32 _mm_sllv_epi32
#define _srlv_epi32 _mm_srlv_epi32
#define _slli_epi32 _mm_slli_epi32
#define _srli_epi32 _mm_srli_epi32

#define _set1_epi32 _mm_set1_epi32
#define _setr_epi32 _mm_setr_epi32
#define _setzero_si _mm_setzero_si128
#define _i32gather_epi32 _mm_i32gather_epi32

#define _cast_128si __m128i
#define _castsi_ps _mm_castsi128_ps
#define _castps_si _mm_castps_si128

#define _unpackhi_ps _mm_unpackhi_ps
#define _unpacklo_ps _mm_unpacklo_ps
#define _movelh_ps _mm_movelh_ps
#define _movehl_ps _mm_movehl_ps

#endif // USE_256_BIT

#define __reg128 __m128i
#define _loadu_si128 _mm_loadu_si128
#define _storeu_si128 _mm_storeu_si128
#define _setzero_si128 _mm_setzero_si128

#endif
