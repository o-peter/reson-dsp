# reson-dsp

![CI](https://github.com/o-peter/reson-dsp/actions/workflows/ci.yml/badge.svg)

`reson` is a small cross-platform C++23 DSP utility library focused on real-time-safe 
audio processing primitives. It provides a clean `std::span`-based API, denormal
handling, and measured performance.

The core library is dependency-free and framework-agnostic: it does not depend
on JUCE, VST3, or any host audio SDK. Callers adapt their own audio buffers to
`std::span<float>` at the API boundary.

## Example

```cpp
#include <reson/reson.hpp>

void process_channel(float* data, std::size_t sample_count) {
    reson::scoped_denormal_disable no_denormals;

    std::span<float> channel {data, sample_count};

    reson::gain(channel, 0.5f);
    reson::soft_clip(channel);
}
```

## Building from source

```bash
git clone https://github.com/o-peter/reson-dsp
cd reson-dsp
cmake --preset release
cmake --build --preset release
```

## Using reson from another CMake project

The recommended source-integration path is `cmake/AddReson.cmake`.

Copy or vendor `cmake/AddReson.cmake` into your project, then include it from
your `CMakeLists.txt`:

```cmake
include(cmake/AddReson.cmake)

add_reson(
    GIT_REPOSITORY https://github.com/o-peter/reson-dsp.git
    GIT_TAG main
)

target_link_libraries(my_target PRIVATE reson::reson)
```

For local development, you can point it at a checked-out copy instead:
```cmake
include(cmake/AddReson.cmake)

add_reson(
    SOURCE_DIR /path/to/reson-dsp
)

target_link_libraries(my_target PRIVATE reson::reson)
```

## Tests and benchmarks

The core `reson` library has no third-party dependencies.

Tests and benchmarks are optional developer targets:

- tests use [GoogleTest](https://github.com/google/googletest)
- benchmarks use [Google Benchmark](https://github.com/google/benchmark)

The project is set up for vcpkg manifest mode, so these dependencies can be
resolved automatically when configuring CMake with the vcpkg toolchain file.
When using the provided presets, set `VCPKG_ROOT` to your vcpkg checkout before configuring.


### Configure and run tests

```bash
export VCPKG_ROOT=/path/to/vcpkg
cmake --preset debug
cmake --build --preset debug
ctest --test-dir build/debug --output-on-failure
```

### Configure and run benchmarks

```bash
export VCPKG_ROOT=/path/to/vcpkg
cmake --preset release-bench
cmake --build --preset release-bench
./build/release-bench/reson_benchmarks
```

Benchmarks should be run from a Release build. The results are intended for
relative comparison between implementations/backends on the same machine, not as
portable performance guarantees.

Results on Macbook Air M1 / Apple Clang 17.0 / Release
```
-------------------------------------------------------------------------------------
Benchmark                           Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------
BM_soft_clip/generic/16           692 ns          694 ns       841852 items_per_second=23.0507M/s
BM_soft_clip/generic/32           724 ns          726 ns       957056 items_per_second=44.0519M/s
BM_soft_clip/generic/64           792 ns          793 ns       879939 items_per_second=80.695M/s
BM_soft_clip/generic/128          929 ns          931 ns       755100 items_per_second=137.46M/s
BM_soft_clip/generic/256         1211 ns         1213 ns       577453 items_per_second=210.981M/s
BM_soft_clip/generic/512         1764 ns         1765 ns       395796 items_per_second=290.164M/s
BM_soft_clip/generic/1024        2880 ns         2877 ns       244366 items_per_second=355.873M/s
BM_soft_clip/generic/2048        5122 ns         5119 ns       136855 items_per_second=400.109M/s
BM_soft_clip/generic/4096        9561 ns         9552 ns        72966 items_per_second=428.818M/s
BM_soft_clip/arm_neon/16          664 ns          666 ns      1048595 items_per_second=24.0274M/s
BM_soft_clip/arm_neon/32          669 ns          670 ns      1043655 items_per_second=47.7508M/s
BM_soft_clip/arm_neon/64          681 ns          682 ns      1021614 items_per_second=93.8081M/s
BM_soft_clip/arm_neon/128         706 ns          708 ns       995690 items_per_second=180.862M/s
BM_soft_clip/arm_neon/256         746 ns          747 ns       933856 items_per_second=342.593M/s
BM_soft_clip/arm_neon/512         830 ns          831 ns       844248 items_per_second=616.085M/s
BM_soft_clip/arm_neon/1024       1005 ns         1006 ns       689390 items_per_second=1.01752G/s
BM_soft_clip/arm_neon/2048       1353 ns         1354 ns       517859 items_per_second=1.51206G/s
BM_soft_clip/arm_neon/4096       2078 ns         2075 ns       343062 items_per_second=1.97424G/s
```
