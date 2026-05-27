#pragma once

#include <span>

namespace reson {

/// Multiplies every sample in place by the specified linear gain factor.
void gain(std::span<float> buffer, float linear_gain) noexcept;

/// Clamps samples in place to the specified range.
///
/// Preconditions:
/// - minimum <= maximum
///
/// Violating this precondition is undefined behavior in release builds.
/// Debug builds assert the precondition.
void hard_clip(std::span<float> buffer, float minimum, float maximum) noexcept;

/// Applies a tanh-style soft clip in place.
///
/// When available, SIMD backends are used with a bounded Padé approximation 
/// to tanh for performance. Results are intended to be perceptually equivalent,
/// not bit-identical to std::tanh.
void soft_clip(std::span<float> buffer) noexcept;

/// Mixes two input buffers into an output buffer with the specified balance.
/// balance = 0.0 means only the first buffer (a) is heard, balance = 1.0
/// means only the second buffer (b) is heard.
/// Preconditions:
/// - a.size() == b.size()
/// - a.size() == out.size()
///
/// Violating these preconditions is undefined behavior in release builds.
/// Debug builds assert the preconditions.
void mix(
    std::span<const float> a,
    std::span<const float> b,
    std::span<float> out,
    float balance
) noexcept;

/// Returns the maximum absolute sample value in the buffer.
[[nodiscard]]
float peak(std::span<const float> buffer) noexcept;

/// Returns the root mean square (RMS) level of the buffer.
[[nodiscard]]
float rms(std::span<const float> buffer) noexcept;

/// One-pole (6dB/octave) low-pass filter with a configurable cutoff frequency.
class one_pole_lowpass {
public:
    one_pole_lowpass(float sample_rate_hz, float cutoff_hz) noexcept;

    /// Processes an entire buffer of input samples and writes the results to the output buffer.
    /// Preconditions:
    /// - input.size() == output.size()
    ///
    /// Violating these preconditions is undefined behavior in release builds.
    /// Debug builds assert the preconditions.
    void process(std::span<const float> input, std::span<float> output) noexcept;

    /// Resets the filter state to the specified value (default is 0.0).
    void reset(float value = 0.0f) noexcept;

    /// Returns the filter's current state.
    [[nodiscard]]
    float state() const noexcept;

private:
    float coefficient_ = 1.0f;
    float state_ = 0.0f;
};

} // namespace reson