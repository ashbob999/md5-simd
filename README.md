# md5-simd
An md5 implementation using **AVX2** instructions.

Built from an [existing md5 implementation](http://www.zedwood.com/article/cpp-md5-function).
Also some SIMD functions took from [https://github.com/crossbowerbt/md5/](https://github.com/crossbowerbt/md5/blob/master/md5_sse.c)

## Building
Simply copy the files in [source/simd](../master/source/simd)

- Requires msvc or gcc.
- Requires at least C++17 to build.
- Requires the computer to have an intel CPU and have supprt for `AVX2`.

For `256-bit` version add `USE_256_BITS` as a compile option.

###### Windows (msvc)
Use `/O2` and `/arch:AVX2` compiler options.

###### Linux (gcc)
Use `-O3` and `-m64 -march=avx2` or equivalent compiler options.

## Usage
Create the class:
```cpp
md5_simd::MD5_SIMD md5;
```


The next thing is to decide whether the inputs are char arrays or strings.

For char arrays:
- create a `char*` `const char*` array with upto 4 or 8 values.
- create a `uint_64t*` array with the length of each input.
```cpp
char* input[4] = {"a", "b", "c", "d"};
uint_64t lengths[4] = {1, 1, 1, 1};
```

For strings:
- create a `std::string[]` array with upto 4 or 8 values.
```cpp
std::string input[4] = {"a", "b", "c", "d"};
```


Next call `MD5_SIMD::calculate<N>` where `N` is the number of inputs to the function.
Passing either the char array and int array, or the string array.
- For `128-bit` version, `N` ranges from 1-4
- For `256-bit` version, `N` ranges from 1-8

This will calculate the md5 hash of the given input strings.

To access the hexdigest for each input string use `MD5_SIMD::hexdigest`, the function takes an index which determines the hash for the given input index.
There are 2 versions:
- One returns a string `std::string hexdigest(int index)`.
- The other takes  a char array as a parameter `void hexdigest(char* str, int index)`, note that the char array must be allocated before use, with a minimum size of 32 chars.

###### Check Zeroes
It also has the ability to check how many leading zeroes a hash has.

```cpp
MD5_SIMD::check_zeroes<N>(int index);
```

The index specifies which hash to check.
`N` is the number of leading zeroes to check for.

###### Input Length Problems
For each call to `MD5_SIMD::calculate` there is a constraint on the lengths the inputs can be.
This is because md5 operates on 512-bit (64-byte) chunks.

- The first input can be any length.
- Each subsequent input must fall within the same 512-bit chunk as the first input.
- if the `first_length % 64 < 56`, then so must all the other inputs.
- otherwise if `first_length % 64 >= 56`, then all the other lengths must either satisfy
    - if `first_length // 64 == current_length // 64`, then `current_length % 64 >= 56`.
    - else `if first_length // 64 == (current_length // 64) + 1`, then `current_length % 64 < 56`.

If any input does not satisfy these checks a runtime error will be produced.

These checks can however be turned off by passing a template bool to calculate: `calculate<N, false>`, a compiler warning will also be given telling you that the lenght checks are disabled.

This can result in incorrect outputs or memory access errors,
but the code can run slightly quicker, 1-10% for doing 1-4 md5 calculations simultaneously, it is not any quicker when doing 8 at a time.

## Running The Tests
- Open the root folder using visual studio.
- Choose one of the build options.

Note: the Linux builds require WSL to be installed and setup for visual studio.

## Benchmarks
`SIMD (Nx)` means it is doing `N` hashes simultaneously.

The non-bracketed values are the `128-bit` version, and the bracketed values are the `256-bit` version.
Original values are the same for both versions.
#### MSVC
|Version  |Time Per Iteration (us)|Speed Multiplier|
|:-------:|:---------------------:|:--------------:|
|Original |7119                   |1x              |
|SIMD (1x)|7452 (9447)            |0.96x (0.75x)   |
|SIMD (2x)|3892 (4791)            |1.83x (1.49x)   |
|SIMD (4x)|2102 (2582)            |3.39x (2.76x)   |
|SIMD (8x)|N/A (1432)             |N/A (4.97x)     |

#### GCC (wsl)
|Version  |Time Per Iteration (us)|Speed Multiplier|
|:-------:|:---------------------:|:--------------:|
|Original |6260                   |1x              |
|SIMD (1x)|7290 (9860)            |0.86x (0.63x)   |
|SIMD (2x)|3776 (5211)            |1.66x (1.2x)    |
|SIMD (4x)|2022 (2731)            |3.1x (2.29x)    |
|SIMD (8x)|N/A (1470)             |N/A (4.26x)     |

## Contents
#### original
Contains the original non-simd md5 class.

#### simd
Contains the simd md5 class.

