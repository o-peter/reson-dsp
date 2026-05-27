#include <reson/dsp.hpp>
#include <reson/denormals.hpp>
#include "cpu_arch.hpp"
#include "generic/soft_clip_generic.hpp"
#include "x86/soft_clip_avx2.hpp"
#include "arm/soft_clip_neon.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

namespace reson {

void gain(std::span<float> buffer, float linear_gain) noexcept {
    for (float& sample : buffer) {
        sample *= linear_gain;
    }
}

void hard_clip(std::span<float> buffer, float minimum, float maximum) noexcept {
    assert(minimum <= maximum);

    for (float& sample : buffer) {
        sample = std::clamp(sample, minimum, maximum);
    }
}

void mix(
    std::span<const float> a,
    std::span<const float> b,
    std::span<float> out,
    float balance
) noexcept {
    assert(a.size() == b.size());
    assert(a.size() == out.size());

    balance = std::clamp(balance, 0.0f, 1.0f);

    const float a_gain = 1.0f - balance;
    const float b_gain = balance;

    for (std::size_t i = 0; i < out.size(); ++i) {
        out[i] = a[i] * a_gain + b[i] * b_gain;
    }
}

float peak(std::span<const float> buffer) noexcept {
    float result = 0.0f;

    for (float sample : buffer) {
        result = std::max(result, std::abs(sample));
    }

    return result;
}

float rms(std::span<const float> buffer) noexcept {
    if (buffer.empty()) {
        return 0.0f;
    }

    double sum = 0.0;

    for (float sample : buffer) {
        sum += static_cast<double>(sample) * static_cast<double>(sample);
    }

    return static_cast<float>(std::sqrt(sum / static_cast<double>(buffer.size())));
}

void soft_clip(std::span<float> buffer) noexcept {
#if defined(RESON_HAS_X86_AVX2)
    detail::avx2::soft_clip(buffer);
#elif defined(RESON_HAS_ARM_NEON)
    detail::neon::soft_clip(buffer);
#else
    detail::generic::soft_clip(buffer);
#endif
}

one_pole_lowpass::one_pole_lowpass(float sample_rate_hz, float cutoff_hz) noexcept {
    if (sample_rate_hz <= 0.0f || cutoff_hz <= 0.0f) {
        coefficient_ = 1.0f;
        return;
    }

    const float nyquist = sample_rate_hz * 0.5f;
    cutoff_hz = std::clamp(cutoff_hz, 1.0f, nyquist);

    coefficient_ = 1.0f - std::exp(
        -2.0f * std::numbers::pi_v<float> * cutoff_hz / sample_rate_hz
    );
}

void one_pole_lowpass::process(
    std::span<const float> input,
    std::span<float> output
) noexcept {
    assert(input.size() == output.size());

    for (std::size_t i = 0; i < input.size(); ++i) {
        state_ = state_ + coefficient_ * (input[i] - state_);
        state_ = snap_to_zero(state_);
        output[i] = state_;
    }
}

void one_pole_lowpass::reset(float value) noexcept {
    state_ = value;
}

float one_pole_lowpass::state() const noexcept {
    return state_;
}

} // namespace reson