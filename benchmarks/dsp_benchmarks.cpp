#include <benchmark/benchmark.h>

#include <reson/denormals.hpp>
#include <reson/dsp.hpp>

#include "generic/soft_clip_generic.hpp"
#include "arm/soft_clip_neon.hpp"
#include "x86/soft_clip_avx2.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
namespace {

std::vector<float> make_input_buffer(std::size_t sample_count) {
    std::vector<float> buffer(sample_count);

    for (std::size_t i = 0; i < sample_count; ++i) {
        const auto phase = static_cast<float>(i % 257) / 128.0f - 1.0f;
        buffer[i] = phase * 3.0f;
    }

    return buffer;
}

void add_common_args(benchmark::Benchmark* benchmark) {
    benchmark
        ->Arg(16)
        ->Arg(32)
        ->Arg(64)
        ->Arg(128)
        ->Arg(256)
        ->Arg(512)
        ->Arg(1024)
        ->Arg(2048)
        ->Arg(4096);
}

using soft_clip_fn = void (*)(std::span<float>) noexcept;

static void BM_soft_clip(benchmark::State& state, soft_clip_fn soft_clip) {
    const auto sample_count = static_cast<std::size_t>(state.range(0));

    const auto input = make_input_buffer(sample_count);
    std::vector<float> buffer(sample_count);

    reson::scoped_denormal_disable no_denormals;

    for (auto _ : state) {
        state.PauseTiming();
        std::ranges::copy(input, buffer.begin());
        state.ResumeTiming();

        soft_clip(buffer);

        benchmark::DoNotOptimize(buffer.data());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(
        state.iterations() * static_cast<std::int64_t>(sample_count)
    );
}

} // namespace

BENCHMARK_CAPTURE(
    BM_soft_clip,
    generic,
    static_cast<soft_clip_fn>(&reson::detail::generic::soft_clip)
)->Apply(add_common_args);

#if defined(RESON_HAS_ARM_NEON)
BENCHMARK_CAPTURE(
    BM_soft_clip,
    arm_neon,
    static_cast<soft_clip_fn>(&reson::detail::neon::soft_clip)
)->Apply(add_common_args);
#endif

#if defined(RESON_HAS_X86_AVX2)
BENCHMARK_CAPTURE(
    BM_soft_clip,
    x86_avx2,
    static_cast<soft_clip_fn>(&reson::detail::avx2::soft_clip)
)->Apply(add_common_args);
#endif

BENCHMARK_MAIN();