#pragma once

#include <cstdint>

#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86_FP)
    #define RESON_HAS_X86_DENORMAL_CONTROL 1
    #include <xmmintrin.h>
#endif

#if (defined(__aarch64__) || defined(__arm64__)) && (defined(__GNUC__) || defined(__clang__))
    #define RESON_HAS_ARM64_DENORMAL_CONTROL 1
#endif

namespace reson {

/// RAII guard that disables denormal/subnormal floating-point handling for
/// the current thread where supported.
///
/// On x86, this enables both FTZ and DAZ:
/// - FTZ flushes subnormal results to zero.
/// - DAZ treats subnormal input operands as zero.
///
/// On ARM64, this enables FPCR flush-to-zero mode.
///
/// This is intended for real-time DSP code, where denormals can cause large
/// performance spikes. The previous floating-point control state is restored
/// when the guard is destroyed.
class scoped_denormal_disable {
public:
    scoped_denormal_disable() noexcept {
#if defined(RESON_HAS_X86_DENORMAL_CONTROL)
        previous_state_ = _mm_getcsr();

        // x86 FTZ and DAZ are enabled by setting bits 15 (FTZ) and 6 (DAZ)
        constexpr std::uint32_t flush_results_to_zero = 1u << 15; // FTZ
        constexpr std::uint32_t treat_inputs_as_zero = 1u << 6;   // DAZ

        _mm_setcsr(
            static_cast<std::uint32_t>(previous_state_) |
            flush_results_to_zero |
            treat_inputs_as_zero
        );

#elif defined(RESON_HAS_ARM64_DENORMAL_CONTROL)
        previous_state_ = read_fpcr();

        // ARM64 FPCR flush-to-zero mode is enabled by setting bit 24 (FZ).
        // This causes both subnormal inputs to be treated as zero and subnormal results to be flushed to zero.
        // (no separate DAZ/FTZ bits like on x86)
        constexpr std::uint64_t flush_to_zero = 1ull << 24;

        write_fpcr(previous_state_ | flush_to_zero);
#endif
    }

    ~scoped_denormal_disable() noexcept {
#if defined(RESON_HAS_X86_DENORMAL_CONTROL)
        _mm_setcsr(static_cast<std::uint32_t>(previous_state_));
#elif defined(RESON_HAS_ARM64_DENORMAL_CONTROL)
        write_fpcr(previous_state_);
#endif
    }

    scoped_denormal_disable(const scoped_denormal_disable&) = delete;
    scoped_denormal_disable& operator=(const scoped_denormal_disable&) = delete;

    scoped_denormal_disable(scoped_denormal_disable&&) = delete;
    scoped_denormal_disable& operator=(scoped_denormal_disable&&) = delete;

private:
#if defined(RESON_HAS_ARM64_DENORMAL_CONTROL)
    static std::uint64_t read_fpcr() noexcept {
        std::uint64_t value = 0;
        asm volatile("mrs %0, fpcr" : "=r"(value));
        return value;
    }

    static void write_fpcr(std::uint64_t value) noexcept {
        asm volatile("msr fpcr, %0" :: "r"(value));
    }
#endif

    std::uint64_t previous_state_ = 0;
};

[[nodiscard]]
constexpr float snap_to_zero(float x) noexcept {
    constexpr float threshold = 1.0e-20f;

    return x > -threshold && x < threshold
        ? 0.0f
        : x;
}

} // namespace reson