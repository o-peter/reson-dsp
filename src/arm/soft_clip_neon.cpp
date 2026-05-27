#include "cpu_arch.hpp"
#if defined(RESON_HAS_ARM_NEON)
#include "arm/soft_clip_neon.hpp"
#include "generic/soft_clip_generic.hpp"
#include <arm_neon.h>
#include <span>
#include <cmath>

namespace reson::detail::neon {

using f32x4 = float32x4_t;

[[nodiscard]]
inline f32x4 splat(float value) noexcept {
    return vdupq_n_f32(value);
}

[[nodiscard]]
inline f32x4 clamp(f32x4 x, float lo, float hi) noexcept {
    return vmaxq_f32(splat(lo), vminq_f32(splat(hi), x));
}

[[nodiscard]]
inline f32x4 reciprocal_nr2(f32x4 x) noexcept {
    // vrecpeq_f32 returns a low-precision reciprocal estimate
    // Refine it twice using Newton-Raphson:
    // r <- r * (2 - x * r)
    f32x4 r = vrecpeq_f32(x);

    r = vmulq_f32(r, vrecpsq_f32(x, r));
    r = vmulq_f32(r, vrecpsq_f32(x, r));
    return r;
}

namespace tanh_pade_approx {

[[nodiscard]] inline f32x4 n0() noexcept { return splat(135135.0f); }
[[nodiscard]] inline f32x4 n1() noexcept { return splat(17325.0f); }
[[nodiscard]] inline f32x4 n2() noexcept { return splat(378.0f); }

[[nodiscard]] inline f32x4 d0() noexcept { return splat(135135.0f); }
[[nodiscard]] inline f32x4 d1() noexcept { return splat(62370.0f); }
[[nodiscard]] inline f32x4 d2() noexcept { return splat(3150.0f); }
[[nodiscard]] inline f32x4 d3() noexcept { return splat(28.0f); }

} // namespace tanh_pade_approx

[[nodiscard]]
inline f32x4 tanh_approx(f32x4 x) noexcept {
    using namespace tanh_pade_approx;

    const f32x4 x2 = vmulq_f32(x, x);

    // numerator = x * (n0 + x^2 * (n1 + x^2 * (n2 + x^2)))
    f32x4 numerator_poly = vaddq_f32(n2(), x2);
    numerator_poly = vfmaq_f32(n1(), x2, numerator_poly);
    numerator_poly = vfmaq_f32(n0(), x2, numerator_poly);

    // denominator = d0 + x^2 * (d1 + x^2 * (d2 + x^2 * d3))
    f32x4 denominator = vfmaq_f32(d2(), x2, d3());
    denominator = vfmaq_f32(d1(), x2, denominator);
    denominator = vfmaq_f32(d0(), x2, denominator);

    const f32x4 numerator = vmulq_f32(x, numerator_poly);
    const f32x4 result = vmulq_f32(numerator, reciprocal_nr2(denominator));

    return clamp(result, -1.0f, 1.0f);
}

void soft_clip(std::span<float> buffer) noexcept {
    const std::size_t simd_width = 4;
    const std::size_t simd_count = buffer.size() / simd_width;

    for (std::size_t i = 0; i < simd_count; ++i) {
        const f32x4 x = vld1q_f32(buffer.data() + i * simd_width);
        const f32x4 y = tanh_approx(x);
        vst1q_f32(buffer.data() + i * simd_width, y);
    }

    if (buffer.size() % simd_width != 0) [[unlikely]] {
        // process remaining samples with scalar fallback
        generic::soft_clip(buffer.subspan(simd_count * simd_width));
    }
}

} // namespace reson::detail::neon

#endif