#pragma once
#include "cpu_arch.hpp"
#if defined(RESON_HAS_X86_AVX2)

#include <span>
#include <immintrin.h>

namespace reson::detail::avx2 {

void soft_clip(std::span<float> buffer) noexcept;

} // namespace reson::detail::avx2

#endif