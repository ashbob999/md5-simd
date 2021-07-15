# md5-simd
An md5 implementation using **AVX2** instructions.

Built from an [existing md5 implementation](http://www.zedwood.com/article/cpp-md5-function).
Also some SIMD functions took from [https://github.com/crossbowerbt/md5/](https://github.com/crossbowerbt/md5/blob/master/md5_sse.c)

## Usage
Simply copy the files in [source/simd](../blob/master/source/simd)

Make sure to compile the source with `/02` (msvc) or `-O3` (gcc) for best performance.

## Benchmarks
`SIMD (Nx)` means it is doing `N` hashes simultaneously.
#### MSVC
|Version  |Time Per Iteration (us)|Speed Multiplier|
|:-------:|:---------------------:|:--------------:|
|Original |6999.275               |1x              |
|SIMD (1x)|9000.407               |0.78x           |
|SIMD (2x)|4895.738               |1.43x           |
|SIMD (4x)|2974.486               |2.35x           |

#### GCC (wsl)
|Version  |Time Per Iteration (us)|Speed Multiplier|
|:-------:|:---------------------:|:--------------:|
|Original |7294.468               |1x              |
|SIMD (1x)|8912.532               |0.82x           |
|SIMD (2x)|4869.745               |1.5x            |
|SIMD (4x)|2984.686               |2.44x           |

## Contents
#### original
Contains the original non-simd md5 class.

#### simd
Contains the simd md5 class.

## Compiling
Open the root folder in Visual Studio, and then select one of the configurations to build.

NOTE: Your computer has to at least support AVX2 instructions.

NOTE: The Linux build configurations require `wsl`.
