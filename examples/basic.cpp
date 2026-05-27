#include <reson/reson.hpp>

#include <array>
#include <iostream>

int main() {
    std::cout << "reson "
              << reson::version_major << "."
              << reson::version_minor << "."
              << reson::version_patch << '\n';

    reson::scoped_denormal_disable no_denormals;

    std::array<float, 8> buffer {
        -1.5f, -1.0f, -0.5f, 0.0f,
         0.25f, 0.5f, 1.0f, 1.5f
    };

    reson::gain(buffer, 0.5f);
    reson::soft_clip(buffer);

    std::cout << "processed buffer:\n";

    for (float sample : buffer) {
        std::cout << "  " << sample << '\n';
    }

    std::cout << "peak: " << reson::peak(buffer) << '\n';
    std::cout << "rms:  " << reson::rms(buffer) << '\n';

    reson::one_pole_lowpass filter { 48000.0f, 1000.0f };

    std::array<float, 8> impulse {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };

    std::array<float, 8> filtered {};

    filter.process(impulse, filtered);

    std::cout << "lowpass impulse response:\n";

    for (float sample : filtered) {
        std::cout << "  " << sample << '\n';
    }

    return 0;
}