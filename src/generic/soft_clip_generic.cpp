#include "generic/soft_clip_generic.hpp"
#include <cmath>
#include <span>

namespace reson::detail::generic {

void soft_clip(std::span<float> buffer) noexcept {
    for (float& sample : buffer) {
        sample = std::tanh(sample);
    }
}

} // namespace reson::detail::generic
