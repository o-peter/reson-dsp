#pragma once
#include "cpu_arch.hpp"
#if defined(RESON_HAS_ARM_NEON)
#include <span>
#include <arm_neon.h>

namespace reson::detail::neon {

void soft_clip(std::span<float> buffer) noexcept;

} // namespace reson::detail::neon

#endif
