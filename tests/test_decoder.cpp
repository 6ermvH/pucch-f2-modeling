#include <gtest/gtest.h>
#include "codec/block_encoder.hpp"
#include "codec/soft_decoder.hpp"

using namespace pucch;

static LLRs perfect_llrs(const Bits& codeword) {
    LLRs llrs(20);
    for (int i = 0; i < 20; ++i) llrs[i] = codeword[i] ? 100.0 : -100.0;
    return llrs;
}

TEST(SoftDecoder, RoundTripAllZeros) {
    for (int n : {2, 4, 6, 8, 11}) {
        Bits data(n, 0);
        Bits cw   = encode(data, n);
        Bits got  = decode(perfect_llrs(cw), n);
        EXPECT_EQ(got, data) << "n=" << n;
    }
}

TEST(SoftDecoder, RoundTripAllOnes) {
    for (int n : {2, 4, 6, 8, 11}) {
        Bits data(n, 1);
        Bits cw  = encode(data, n);
        Bits got = decode(perfect_llrs(cw), n);
        EXPECT_EQ(got, data) << "n=" << n;
    }
}

TEST(SoftDecoder, RoundTripAllPatternsN4) {
    constexpr int n = 4;
    for (int d = 0; d < (1 << n); ++d) {
        Bits data(n);
        for (int j = 0; j < n; ++j) data[j] = (d >> j) & 1;

        Bits cw  = encode(data, n);
        Bits got = decode(perfect_llrs(cw), n);
        EXPECT_EQ(got, data) << "d=" << d;
    }
}

TEST(SoftDecoder, RoundTripSamplesN11) {
    constexpr int n = 11;
    for (int d : {0, 1, 42, 100, 1024, 2047}) {
        Bits data(n);
        for (int j = 0; j < n; ++j) data[j] = (d >> j) & 1;

        Bits cw  = encode(data, n);
        Bits got = decode(perfect_llrs(cw), n);
        EXPECT_EQ(got, data) << "d=" << d;
    }
}

TEST(SoftDecoder, OutputSizeEqualsNBits) {
    for (int n : {2, 4, 6, 8, 11}) {
        LLRs llrs(20, 1.0);
        EXPECT_EQ(decode(llrs, n).size(), static_cast<std::size_t>(n));
    }
}

TEST(SoftDecoder, WrongLLRSizeThrows) {
    EXPECT_THROW(decode(LLRs(19, 0.0), 4), std::invalid_argument);
    EXPECT_THROW(decode(LLRs(21, 0.0), 4), std::invalid_argument);
}

TEST(SoftDecoder, PicksHigherMetricCandidate) {
    constexpr int n = 2;
    Bits cw_10 = encode({1, 0}, n);
    Bits cw_01 = encode({0, 1}, n);

    EXPECT_EQ(decode(perfect_llrs(cw_10), n), (Bits{1, 0}));
    EXPECT_EQ(decode(perfect_llrs(cw_01), n), (Bits{0, 1}));
}
