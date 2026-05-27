#include <reson/denormals.hpp>
#include <reson/dsp.hpp>

#include <gtest/gtest.h>

#include <array>
#include <cmath>

namespace {

constexpr float default_epsilon = 1.0e-5f;

void expect_nearly_equal(float actual, float expected, float epsilon = default_epsilon) {
    EXPECT_NEAR(actual, expected, epsilon);
}

} // namespace

TEST(Gain, MultipliesEverySample) {
    std::array<float, 4> buffer {1.0f, -2.0f, 3.0f, -4.0f};

    reson::gain(buffer, 2.0f);

    expect_nearly_equal(buffer[0], 2.0f);
    expect_nearly_equal(buffer[1], -4.0f);
    expect_nearly_equal(buffer[2], 6.0f);
    expect_nearly_equal(buffer[3], -8.0f);
}

TEST(HardClip, ClampsSamplesToRange) {
    std::array<float, 5> buffer {-2.0f, -0.5f, 0.0f, 0.5f, 2.0f};

    reson::hard_clip(buffer, -1.0f, 1.0f);

    expect_nearly_equal(buffer[0], -1.0f);
    expect_nearly_equal(buffer[1], -0.5f);
    expect_nearly_equal(buffer[2], 0.0f);
    expect_nearly_equal(buffer[3], 0.5f);
    expect_nearly_equal(buffer[4], 1.0f);
}

TEST(SoftClip, AppliesTanhToEverySample) {
    const std::array<float, 9> input {
        -2.0f, -1.5f, -1.0f, -0.5f, 0.0f, 0.5f, 1.0f, 1.5f, 2.0f
    };

    auto buffer = input;

    reson::soft_clip(buffer);

    constexpr float allowed_error = 1.0e-5f;

    for (std::size_t i = 0; i < buffer.size(); ++i) {
        EXPECT_NEAR(buffer[i], std::tanh(input[i]), allowed_error)
            << "at index " << i << ", input = " << input[i];
    }
}

TEST(SoftClip, OutputIsBounded) {
    std::array<float, 9> buffer {-100000.0f, -100.0f, -10.0f, -0.0f, 0.0f, 0.000001f, 10.0f, 100.0f, 100000.0f};

    reson::soft_clip(buffer);

    for (float x : buffer) {
        EXPECT_GE(x, -1.0f);
        EXPECT_LE(x, 1.0f);
    }
}

TEST(Mix, BlendsTwoBuffers) {
    std::array<float, 3> a {1.0f, 1.0f, 1.0f};
    std::array<float, 3> b {3.0f, 3.0f, 3.0f};
    std::array<float, 3> out {};

    reson::mix(a, b, out, 0.25f);

    expect_nearly_equal(out[0], 1.5f);
    expect_nearly_equal(out[1], 1.5f);
    expect_nearly_equal(out[2], 1.5f);
}

TEST(Peak, ReturnsMaximumAbsoluteSample) {
    std::array<float, 5> buffer {-0.1f, 0.2f, -3.0f, 0.4f, 2.0f};

    expect_nearly_equal(reson::peak(buffer), 3.0f);
}

TEST(Rms, ReturnsRootMeanSquare) {
    std::array<float, 4> buffer {1.0f, 1.0f, 1.0f, 1.0f};

    expect_nearly_equal(reson::rms(buffer), 1.0f);
}

TEST(Rms, ReturnsZeroForEmptyBuffer) {
    std::array<float, 0> empty {};

    expect_nearly_equal(reson::rms(empty), 0.0f);
}

TEST(SnapToZero, FlushesTinyValuesToZero) {
    EXPECT_FLOAT_EQ(reson::snap_to_zero(1.0e-30f), 0.0f);
    EXPECT_FLOAT_EQ(reson::snap_to_zero(-1.0e-30f), 0.0f);
}

TEST(SnapToZero, PreservesNormalValues) {
    EXPECT_FLOAT_EQ(reson::snap_to_zero(1.0e-3f), 1.0e-3f);
    EXPECT_FLOAT_EQ(reson::snap_to_zero(-1.0e-3f), -1.0e-3f);
}

TEST(Denormals, ScopedDenormalDisableConstructs) {
    reson::scoped_denormal_disable guard;
    static_cast<void>(guard);

    SUCCEED();
}

TEST(OnePoleLowpass, StepResponseStartsBetweenZeroAndOne) {
    reson::one_pole_lowpass filter {48000.0f, 1000.0f};

    std::array<float, 1> buffer {1.0f};
    filter.process(buffer, buffer);
    const float first = buffer[0];
    EXPECT_GT(first, 0.0f);
    EXPECT_LT(first, 1.0f);
}

TEST(OnePoleLowpass, OutputDecaysAfterInputReturnsToZero) {
    reson::one_pole_lowpass filter {48000.0f, 1000.0f};

    std::array<float, 1> input {1.0f};
    std::array<float, 1> output {};
    filter.process(input, output);
    const float first = output[0];

    input[0] = 0.0f;
    filter.process(input, output);
    const float second = output[0];

    EXPECT_GT(second, 0.0f);
    EXPECT_LT(second, first);
}
