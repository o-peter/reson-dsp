#pragma once

#if defined(__AVX2__) || defined(_M_AVX2)
    #define RESON_HAS_X86_AVX2 1
    #if defined (__FMA__)
        #define RESON_HAS_X86_FMA 1
    #endif
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
    #define RESON_HAS_ARM_NEON 1
#else
    #define RESON_GENERIC_CPU 1
#endif