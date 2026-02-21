#include <gtest/gtest.h>
#include "codec/block_encoder.hpp"
#include "codec/generator_matrix.hpp"

using namespace pucch;

// ── Вспомогательные утилиты ───────────────────────────────────────────────────

// Возвращает n-й столбец матрицы G в виде Bits (последние n_cols столбцов,
// конкретно столбец с индексом col_offset(n_bits) + local_col).
static Bits column(int global_col) {
    Bits col(20);
    for (int i = 0; i < 20; ++i) col[i] = G[i][global_col];
    return col;
}

static Bits zeros(int n) { return Bits(n, 0); }
static Bits unit(int n, int pos) { Bits b(n, 0); b[pos] = 1; return b; }

// ── Тесты ─────────────────────────────────────────────────────────────────────

// Нулевой вектор данных → нулевое кодовое слово для всех допустимых n.
TEST(BlockEncoder, ZeroInputGivesZeroCodeword) {
    for (int n : {2, 4, 6, 8, 11}) {
        EXPECT_EQ(encode(zeros(n), n), Bits(20, 0)) << "n=" << n;
    }
}

// Единичный вектор e_j → j-й активный столбец матрицы G.
TEST(BlockEncoder, UnitVectorGivesMatrixColumn) {
    for (int n : {2, 4, 6, 8, 11}) {
        int offset = col_offset(n);
        for (int j = 0; j < n; ++j) {
            EXPECT_EQ(encode(unit(n, j), n), column(offset + j))
                << "n=" << n << ", j=" << j;
        }
    }
}

// Конкретные кодовые слова для n=2, проверяем вручную.
TEST(BlockEncoder, KnownVectorsN2) {
    // data=[1,0] → столбец 11
    EXPECT_EQ(encode({1, 0}, 2), column(11));
    // data=[0,1] → столбец 12
    EXPECT_EQ(encode({0, 1}, 2), column(12));
    // data=[1,1] → xor столбцов 11 и 12
    Bits expected(20);
    for (int i = 0; i < 20; ++i) expected[i] = G[i][11] ^ G[i][12];
    EXPECT_EQ(encode({1, 1}, 2), expected);
}

// Линейность: encode(a XOR b) == encode(a) XOR encode(b).
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

// Размер выходного кодового слова всегда 20.
TEST(BlockEncoder, OutputSizeIsAlways20) {
    for (int n : {2, 4, 6, 8, 11}) {
        EXPECT_EQ(encode(zeros(n), n).size(), 20u) << "n=" << n;
    }
}

// Несовпадение размера данных с n_bits → исключение.
TEST(BlockEncoder, WrongDataSizeThrows) {
    EXPECT_THROW(encode({1, 0, 1}, 2), std::invalid_argument);
}
