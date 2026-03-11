#include <gtest/gtest.h>
#include "codec/block_encoder.hpp"
#include "codec/generator_matrix.hpp"

using namespace pucch;

static Bits column(int global_col) {
    Bits col(20);
    for (int i = 0; i < 20; ++i) col[i] = G[i][global_col];
    return col;
}

static Bits zeros(int n) { return Bits(n, 0); }
static Bits unit(int n, int pos) { Bits b(n, 0); b[pos] = 1; return b; }

TEST(BlockEncoder, ZeroInputGivesZeroCodeword) {
    for (int n : {2, 4, 6, 8, 11}) {
        EXPECT_EQ(encode(zeros(n), n), Bits(20, 0)) << "n=" << n;
    }
}

TEST(BlockEncoder, UnitVectorGivesMatrixColumn) {
    for (int n : {2, 4, 6, 8, 11}) {
        int offset = col_offset(n);
        for (int j = 0; j < n; ++j) {
            EXPECT_EQ(encode(unit(n, j), n), column(offset + j))
                << "n=" << n << ", j=" << j;
        }
    }
}

TEST(BlockEncoder, KnownVectorsN2) {
    EXPECT_EQ(encode({1, 0}, 2), column(11));
    EXPECT_EQ(encode({0, 1}, 2), column(12));
    Bits expected(20);
    for (int i = 0; i < 20; ++i) expected[i] = G[i][11] ^ G[i][12];
    EXPECT_EQ(encode({1, 1}, 2), expected);
}

TEST(BlockEncoder, Linearity) {
    Bits a = {1, 0, 1, 1};
    Bits b = {0, 1, 1, 0};
    Bits axb = {1, 1, 0, 1};

    Bits ea = encode(a, 4);
    Bits eb = encode(b, 4);
    Bits eaxb = encode(axb, 4);

    Bits xored(20);
    for (int i = 0; i < 20; ++i) xored[i] = ea[i] ^ eb[i];
    EXPECT_EQ(xored, eaxb);
}

TEST(BlockEncoder, OutputSizeIsAlways20) {
    for (int n : {2, 4, 6, 8, 11}) {
        EXPECT_EQ(encode(zeros(n), n).size(), 20u) << "n=" << n;
    }
}

TEST(BlockEncoder, WrongDataSizeThrows) {
    EXPECT_THROW(encode({1, 0, 1}, 2), std::invalid_argument);
}
