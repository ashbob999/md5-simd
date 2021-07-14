# md5-simd
An md5 implementation using **SSE/AVX** instructions.

Built from an [existing md5 implementation](http://www.zedwood.com/article/cpp-md5-function).
Also some SIMD functions took from [https://github.com/crossbowerbt/md5/](https://github.com/crossbowerbt/md5/blob/master/md5_sse.c)

## Usage
Simply copy the files in [source/simd](../blob/master/source/simd)

Make sure to compile the source with `/02` (msvc) or `-O3` (gcc) for best performance.

## Contents
#### original
Contains the original non-simd md5 class.

#### simd
Contains the simd md5 class.

## Compiling
Open the root folder in Visual Studio, and then select one of the configurations to build.

NOTE: Your computer has to at least support AVX instructions.

NOTE: The Linux build configurations require `wsl`.
