#pragma once

#include <span>

namespace reson::detail::generic {

void soft_clip(std::span<float> buffer) noexcept;

} // namespace reson::detail::generic
