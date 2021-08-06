# md5-simd
An md5 implementation using **AVX2** instructions.

Built from an [existing md5 implementation](http://www.zedwood.com/article/cpp-md5-function).
Also some SIMD functions took from [https://github.com/crossbowerbt/md5/](https://github.com/crossbowerbt/md5/blob/master/md5_sse.c)

## Usage
Simply copy the files in [source/simd](../master/source/simd)

Make sure to compile the source with `/02` (msvc) or `-O3` (gcc) for best performance.

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

## Compiling
Open the root folder in Visual Studio, and then select one of the configurations to build.

NOTE: Your computer has to at least support AVX2 instructions.

NOTE: The Linux build configurations require `wsl`.
