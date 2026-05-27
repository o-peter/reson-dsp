cmake_minimum_required(VERSION 3.25)

project(
    reson-dsp
    VERSION 0.1.0
    DESCRIPTION "Small C++23 real-time-safe DSP primitives"
    LANGUAGES CXX
)

option(RESON_BUILD_EXAMPLES "Build examples" OFF)
option(RESON_BUILD_TESTS "Build tests" OFF)
option(RESON_BUILD_BENCHMARKS "Build benchmarks" OFF)

add_library(reson)
add_library(reson::reson ALIAS reson)

target_sources(reson
    PRIVATE
        src/dsp.cpp
        src/cpu_arch.hpp
        src/generic/soft_clip_generic.cpp
        src/generic/soft_clip_generic.hpp
        src/arm/soft_clip_neon.cpp
        src/arm/soft_clip_neon.hpp
        src/x86/soft_clip_avx2.cpp
        src/x86/soft_clip_avx2.hpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/reson/denormals.hpp
            include/reson/dsp.hpp
            include/reson/reson.hpp
            include/reson/version.hpp
)

target_compile_features(reson PUBLIC cxx_std_23)

target_include_directories(reson
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (MSVC)
    target_compile_options(reson PRIVATE /W4 /permissive-)
else()
    target_compile_options(reson PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (RESON_BUILD_EXAMPLES)
    add_executable(reson_basic
        examples/basic.cpp
    )

    target_link_libraries(reson_basic PRIVATE reson::reson)

    set_target_properties(reson_basic PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
    )
endif()

if (RESON_BUILD_TESTS)
    enable_testing()

    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    add_executable(reson_tests
        tests/dsp_tests.cpp
    )

    target_link_libraries(reson_tests
        PRIVATE
            reson::reson
            GTest::gtest_main
    )

    gtest_discover_tests(reson_tests)

    set_target_properties(reson_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )
endif()

if (RESON_BUILD_BENCHMARKS)
    find_package(benchmark CONFIG REQUIRED)

    add_executable(reson_benchmarks
        benchmarks/dsp_benchmarks.cpp
    )

    target_link_libraries(reson_benchmarks
        PRIVATE
            reson::reson
            benchmark::benchmark
    )

    target_include_directories(reson_benchmarks
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
endif()