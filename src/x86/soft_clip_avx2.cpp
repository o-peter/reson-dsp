#include "cpu_arch.hpp"
#if defined(RESON_HAS_X86_AVX2)
#include "x86/soft_clip_avx2.hpp"
#include "generic/soft_clip_generic.hpp"
#include <immintrin.h>
#include <span>
#include <cmath>

namespace reson::detail::avx2 {

using f32x8 = __m256;

[[nodiscard]]
inline f32x8 splat(float value) noexcept {
    return _mm256_set1_ps(value);
}

[[nodiscard]]
inline f32x8 clamp(f32x8 x, float lo, float hi) noexcept {
    return _mm256_max_ps(splat(lo), _mm256_min_ps(splat(hi), x));
}

[[nodiscard]]
inline f32x8 mul_add(f32x8 a, f32x8 b, f32x8 c) noexcept {
#if defined(RESON_HAS_X86_FMA)
    return _mm256_fmadd_ps(a, b, c); // a * b + c
#else
    return _mm256_add_ps(_mm256_mul_ps(a, b), c);
#endif
}

[[nodiscard]]
inline f32x8 nmul_add(f32x8 a, f32x8 b, f32x8 c) noexcept {
#if defined(RESON_HAS_X86_FMA)
    return _mm256_fnmadd_ps(a, b, c); // -(a * b) + c
#else
    return _mm256_sub_ps(c, _mm256_mul_ps(a, b));
#endif
}

[[nodiscard]]
inline f32x8 reciprocal_nr2(f32x8 x) noexcept {
    const f32x8 two = splat(2.0f);
    f32x8 r = _mm256_rcp_ps(x);

    // _mm256_rcp_ps returns a low-precision reciprocal estimate
    // Refine it twice using Newton-Raphson:
    // r <- r * (2 - x * r)
    r = _mm256_mul_ps(r, nmul_add(x, r, two));
    r = _mm256_mul_ps(r, nmul_add(x, r, two));

    return r;
}

namespace tanh_pade_approx {

[[nodiscard]] inline f32x8 n0() noexcept { return splat(135135.0f); }
[[nodiscard]] inline f32x8 n1() noexcept { return splat(17325.0f); }
[[nodiscard]] inline f32x8 n2() noexcept { return splat(378.0f); }

[[nodiscard]] inline f32x8 d0() noexcept { return splat(135135.0f); }
[[nodiscard]] inline f32x8 d1() noexcept { return splat(62370.0f); }
[[nodiscard]] inline f32x8 d2() noexcept { return splat(3150.0f); }
[[nodiscard]] inline f32x8 d3() noexcept { return splat(28.0f); }

} // namespace tanh_pade_approx

[[nodiscard]]
inline f32x8 tanh_approx(f32x8 x) noexcept {
    using namespace tanh_pade_approx;

    const f32x8 x2 = _mm256_mul_ps(x, x);

    // numerator = x * (n0 + x^2 * (n1 + x^2 * (n2 + x^2)))
    f32x8 numerator_poly = _mm256_add_ps(n2(), x2);
    numerator_poly = mul_add(x2, numerator_poly, n1());
    numerator_poly = mul_add(x2, numerator_poly, n0());

    // denominator = d0 + x^2 * (d1 + x^2 * (d2 + x^2 * d3))
    f32x8 denominator = mul_add(x2, d3(), d2());
    denominator = mul_add(x2, denominator, d1());
    denominator = mul_add(x2, denominator, d0());

    const f32x8 numerator = _mm256_mul_ps(x, numerator_poly);
    const f32x8 result = _mm256_mul_ps(numerator, reciprocal_nr2(denominator));

    return clamp(result, -1.0f, 1.0f);
}

void soft_clip(std::span<float> buffer) noexcept {
    constexpr std::size_t simd_width = 8;
    const std::size_t simd_count = buffer.size() / simd_width;

    for (std::size_t i = 0; i < simd_count; ++i) {
        const f32x8 x = _mm256_loadu_ps(buffer.data() + i * simd_width);
        const f32x8 y = tanh_approx(x);
        _mm256_storeu_ps(buffer.data() + i * simd_width, y);
    }

    if (buffer.size() % simd_width != 0) [[unlikely]] {
        // process remaining samples with scalar fallback
        generic::soft_clip(buffer.subspan(simd_count * simd_width));
    }
}

} // namespace reson::detail::avx2

#endif