
#include "source/original/md5-original.h"
#include "source/simd/md5-simd.h"

#include <iostream>

#include <chrono>
#include <iomanip>

using namespace std;

constexpr int TEST_STRING_LENGTH = 43;
constexpr int RUN_INIT_STRING_LENGTH = 6;
constexpr int RUN_ZERO_COUNT = 4;
constexpr int RUN_RESULT = 31556;

constexpr char char_map[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

inline int digit_count(int number)
{
	if (number < 10) return 1;
	if (number < 100) return 2;
	if (number < 1000) return 3;
	if (number < 10000) return 4;
	if (number < 100000) return 5;
	if (number < 1000000) return 6;
	if (number < 10000000) return 7;
	if (number < 100000000) return 8;
	if (number < 1000000000) return 9;
	return 10;
}

bool test_original()
{
	const char* text = "The quick brown fox jumps over the lazy dog";

	char buf[33];
	buf[32] = '\0';

	MD5 md5;

	md5.calculate(text, TEST_STRING_LENGTH);

	md5.hexdigest(buf);

	if (strcmp(buf, "9e107d9d372bb6826bd81d3542a419d6") == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool test_simd()
{
	const char* text = "The quick brown fox jumps over the lazy dog";

	MD5_SIMD md5;

	const char* arr[4] = { text, text, text, text };
	uint64_t lengths[4] = { TEST_STRING_LENGTH, TEST_STRING_LENGTH, TEST_STRING_LENGTH, TEST_STRING_LENGTH };

	md5.calculate<4>(arr, lengths);

	char buf[33];
	buf[32] = '\0';

	md5.hexdigest(buf, 0);

	if (strcmp(buf, "9e107d9d372bb6826bd81d3542a419d6") == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int run_original()
{
	char input_buffer[100];
	char output_buffer[32];

	memcpy(input_buffer, "abcdef", RUN_INIT_STRING_LENGTH);

	int i = 1;

	MD5 md5;

	while (true)
	{
		int digits = digit_count(i);

		int index = digits + RUN_INIT_STRING_LENGTH - 1;

		int num = i;

		while (num > 0)
		{
			input_buffer[index] = char_map[num % 10];
			num /= 10;
			index--;
		}

		int count = 0;

		md5.calculate(input_buffer, RUN_INIT_STRING_LENGTH + digits);

		md5.hexdigest(output_buffer);
		//cout << output_buffer << endl;

		for (int idx = 0; idx < RUN_ZERO_COUNT; idx++)
		{
			if (output_buffer[idx] == '0')
			{
				count++;
				if (count >= RUN_ZERO_COUNT)
				{
					return i;
				}
			}
		}

		i++;
	}
}

int run_simd_1x()
{
	char* buffers[1];
	buffers[0] = new char[128];

	const char* buffer = "abcdef";

	int n = 1;

	MD5_SIMD md5;

	while (true)
	{
		uint64_t lengths[1];

		int num = n;
		int digits = digit_count(num);

		lengths[0] = RUN_INIT_STRING_LENGTH + digits;

		buffers[0][RUN_INIT_STRING_LENGTH + digits] = '\0';
		memcpy(buffers[0], buffer, RUN_INIT_STRING_LENGTH);

		int index = RUN_INIT_STRING_LENGTH + digits - 1;
		while (num > 0)
		{
			buffers[0][index] = char_map[num % 10];
			num /= 10;
			index--;
		}

		char res[32];

		md5.calculate<1>(buffers, lengths);

		if (md5.check_zeroes<RUN_ZERO_COUNT>(0))
		{
			delete[] buffers[0];
			return n;
		}

		n++;
	}

	delete[] buffers[0];
}

int run_simd_2x()
{
	char* buffers[2];
	buffers[0] = new char[128];
	buffers[1] = new char[128];

	const char* buffer = "abcdef";

	int n = 1;

	MD5_SIMD md5;

	while (true)
	{

		uint64_t lengths[2];

		for (int i = 0; i < 2; i++)
		{
			int num = n + i;
			int digits = digit_count(num);

			lengths[i] = RUN_INIT_STRING_LENGTH + digits;

			buffers[i][RUN_INIT_STRING_LENGTH + digits] = '\0';
			memcpy(buffers[i], buffer, RUN_INIT_STRING_LENGTH);

			int index = RUN_INIT_STRING_LENGTH + digits - 1;
			while (num > 0)
			{
				buffers[i][index] = char_map[num % 10];
				num /= 10;
				index--;
			}
		}

		char res[32];

		md5.calculate<2>(buffers, lengths);

		for (int buffer_index = 0; buffer_index < 2; buffer_index++)
		{
			if (md5.check_zeroes<RUN_ZERO_COUNT>(buffer_index))
			{
				delete[] buffers[0];
				delete[] buffers[1];

				return n + buffer_index;
			}
		}

		n += 2;
	}

	delete[] buffers[0];
	delete[] buffers[1];
}

int run_simd_4x()
{
	char* buffers[4];
	buffers[0] = new char[128];
	buffers[1] = new char[128];
	buffers[2] = new char[128];
	buffers[3] = new char[128];

	const char* buffer = "abcdef";

	int n = 1;

	MD5_SIMD md5;

	while (true)
	{

		uint64_t lengths[4];

		for (int i = 0; i < 4; i++)
		{
			int num = n + i;
			int digits = digit_count(num);

			lengths[i] = RUN_INIT_STRING_LENGTH + digits;

			buffers[i][RUN_INIT_STRING_LENGTH + digits] = '\0';
			memcpy(buffers[i], buffer, RUN_INIT_STRING_LENGTH);

			int index = RUN_INIT_STRING_LENGTH + digits - 1;
			while (num > 0)
			{
				buffers[i][index] = char_map[num % 10];
				num /= 10;
				index--;
			}
		}

		char res[32];

		md5.calculate<4>(buffers, lengths);

		for (int buffer_index = 0; buffer_index < 4; buffer_index++)
		{
			if (md5.check_zeroes<RUN_ZERO_COUNT>(buffer_index))
			{
				delete[] buffers[0];
				delete[] buffers[1];
				delete[] buffers[2];
				delete[] buffers[3];

				return n + buffer_index;
			}
		}

		n += 4;
	}

	delete[] buffers[0];
	delete[] buffers[1];
	delete[] buffers[2];
	delete[] buffers[3];
}

#ifdef USE_256_BITS
int run_simd_8x()
{
	char* buffers[8];
	buffers[0] = new char[128];
	buffers[1] = new char[128];
	buffers[2] = new char[128];
	buffers[3] = new char[128];
	buffers[4] = new char[128];
	buffers[5] = new char[128];
	buffers[6] = new char[128];
	buffers[7] = new char[128];

	const char* buffer = "abcdef";

	int n = 1;

	MD5_SIMD md5;

	while (true)
	{

		uint64_t lengths[8];

		for (int i = 0; i < 8; i++)
		{
			int num = n + i;
			int digits = digit_count(num);

			lengths[i] = RUN_INIT_STRING_LENGTH + digits;

			buffers[i][RUN_INIT_STRING_LENGTH + digits] = '\0';
			memcpy(buffers[i], buffer, RUN_INIT_STRING_LENGTH);

			int index = RUN_INIT_STRING_LENGTH + digits - 1;
			while (num > 0)
			{
				buffers[i][index] = char_map[num % 10];
				num /= 10;
				index--;
			}
		}

		char res[32];

		md5.calculate<8>(buffers, lengths);

		for (int buffer_index = 0; buffer_index < 8; buffer_index++)
		{
			if (md5.check_zeroes<RUN_ZERO_COUNT>(buffer_index))
			{
				delete[] buffers[0];
				delete[] buffers[1];
				delete[] buffers[2];
				delete[] buffers[3];
				delete[] buffers[4];
				delete[] buffers[5];
				delete[] buffers[6];
				delete[] buffers[7];

				return n + buffer_index;
			}
		}

		n += 4;
	}

	delete[] buffers[0];
	delete[] buffers[1];
	delete[] buffers[2];
	delete[] buffers[3];
	delete[] buffers[4];
	delete[] buffers[5];
	delete[] buffers[6];
	delete[] buffers[7];
}
#endif

double time(int reps, int(*fn)())
{
	auto st = chrono::steady_clock::now();

	for (int i = 0; i < reps; i++)
	{
		auto r = fn();
	}

	auto et = chrono::steady_clock::now();

	auto elapsed = et - st;

	double tp = chrono::duration_cast<chrono::nanoseconds>(elapsed).count() / (double) reps;

	double t = 1e-3 * tp;

	return t;
}

int main()
{
	// test for correct hashes
	bool test_original_result = test_original();
	bool test_simd_result = test_simd();

	cout << "test: original => " << (test_original_result ? "succeeded" : "failed") << endl;
	cout << "test: simd => " << (test_simd_result ? "succeeded" : "failed") << endl;

	if (!test_original_result || !test_simd_result)
	{
		return 0;
	}

	cout << endl;

	// test for correct running
	bool run_original_result = run_original() == RUN_RESULT;
	bool run_simd_1x_result = run_simd_4x() == RUN_RESULT;
	bool run_simd_2x_result = run_simd_4x() == RUN_RESULT;
	bool run_simd_4x_result = run_simd_4x() == RUN_RESULT;
#ifdef USE_256_BITS
	bool run_simd_8x_result = run_simd_8x() == RUN_RESULT;
#endif

	cout << "run: original => " << (run_original_result ? "succeeded" : "failed") << endl;
	cout << "run: simd 1x => " << (run_simd_1x_result ? "succeeded" : "failed") << endl;
	cout << "run: simd 2x => " << (run_simd_2x_result ? "succeeded" : "failed") << endl;
	cout << "run: simd 4x => " << (run_simd_4x_result ? "succeeded" : "failed") << endl;
#ifdef USE_256_BITS
	cout << "run: simd 8x => " << (run_simd_8x_result ? "succeeded" : "failed") << endl;
#endif

#ifdef USE_256_BITS
	if (!run_original_result || !run_simd_1x_result || !run_simd_2x_result || !run_simd_4x_result || !run_simd_8x_result)
#else
	if (!run_original_result || !run_simd_1x_result || !run_simd_2x_result || !run_simd_4x_result)
#endif
	{
		return 0;
	}

	cout << endl;

	// time the functions
	int reps = 10;

	double time_original = time(reps, run_original);
	double time_simd_1x = time(reps, run_simd_1x);
	double time_simd_2x = time(reps, run_simd_2x);
	double time_simd_4x = time(reps, run_simd_4x);
#ifdef USE_256_BITS
	double time_simd_8x = time(reps, run_simd_8x);
#endif

	cout << "time: original => " << fixed << setprecision(3) << time_original << "us" << endl;
	cout << "time: simd 1x => " << fixed << setprecision(3) << time_simd_1x << "us" << endl;
	cout << "time: simd 2x => " << fixed << setprecision(3) << time_simd_2x << "us" << endl;
	cout << "time: simd 4x => " << fixed << setprecision(3) << time_simd_4x << "us" << endl;
#ifdef USE_256_BITS
	cout << "time: simd 8x => " << fixed << setprecision(3) << time_simd_8x << "us" << endl;
#endif

	return 0;
}
