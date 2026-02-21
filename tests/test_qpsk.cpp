#include <gtest/gtest.h>
#include "modem/qpsk_modulator.hpp"
#include "modem/qpsk_demodulator.hpp"

#include <cmath>

using namespace pucch;

static constexpr double INV_SQRT2 = 0.7071067811865476;
static constexpr double EPS = 1e-10;

// ── Модулятор ─────────────────────────────────────────────────────────────────

TEST(QpskModulator, KnownSymbolValues) {
    // (0,0) → (-1/√2, -1/√2)
    auto s = modulate({0, 0});
    ASSERT_EQ(s.size(), 1u);
    EXPECT_NEAR(s[0].real(), -INV_SQRT2, EPS);
    EXPECT_NEAR(s[0].imag(), -INV_SQRT2, EPS);

    // (1,1) → (+1/√2, +1/√2)
    s = modulate({1, 1});
    EXPECT_NEAR(s[0].real(), +INV_SQRT2, EPS);
    EXPECT_NEAR(s[0].imag(), +INV_SQRT2, EPS);

    // (1,0) → (+1/√2, -1/√2)
    s = modulate({1, 0});
    EXPECT_NEAR(s[0].real(), +INV_SQRT2, EPS);
    EXPECT_NEAR(s[0].imag(), -INV_SQRT2, EPS);

    // (0,1) → (-1/√2, +1/√2)
    s = modulate({0, 1});
    EXPECT_NEAR(s[0].real(), -INV_SQRT2, EPS);
    EXPECT_NEAR(s[0].imag(), +INV_SQRT2, EPS);
}

TEST(QpskModulator, SymbolEnergyIsOne) {
    for (auto bits : std::vector<Bits>{{0,0},{0,1},{1,0},{1,1}}) {
        auto s = modulate(bits);
        double energy = s[0].real() * s[0].real() + s[0].imag() * s[0].imag();
        EXPECT_NEAR(energy, 1.0, EPS);
    }
}

TEST(QpskModulator, OutputSizeIsHalfInput) {
    Bits bits(20, 0);
    EXPECT_EQ(modulate(bits).size(), 10u);
}

TEST(QpskModulator, OddSizeThrows) {
    EXPECT_THROW(modulate({0, 1, 0}), std::invalid_argument);
}

// ── Демодулятор ───────────────────────────────────────────────────────────────

TEST(QpskDemodulator, LLREqualsCoordinate) {
    Symbols symbols = {{0.5, -0.3}, {-0.7, 0.9}};
    LLRs llrs = demodulate(symbols);

    ASSERT_EQ(llrs.size(), 4u);
    EXPECT_DOUBLE_EQ(llrs[0],  0.5);   // Re(y[0])
    EXPECT_DOUBLE_EQ(llrs[1], -0.3);   // Im(y[0])
    EXPECT_DOUBLE_EQ(llrs[2], -0.7);   // Re(y[1])
    EXPECT_DOUBLE_EQ(llrs[3],  0.9);   // Im(y[1])
}

TEST(QpskDemodulator, OutputSizeIsTwiceInput) {
    Symbols symbols(10);
    EXPECT_EQ(demodulate(symbols).size(), 20u);
}

// ── Совместная проверка: знаки LLR согласованы с маппингом ───────────────────

// Бит=1 → положительная координата → LLR > 0.
// Бит=0 → отрицательная координата → LLR < 0.
TEST(QpskRoundTrip, LLRSignConsistency) {
    for (uint8_t b0 : {0, 1}) {
        for (uint8_t b1 : {0, 1}) {
            auto sym  = modulate({b0, b1});
            auto llrs = demodulate(sym);
            ASSERT_EQ(llrs.size(), 2u);

            if (b0 == 1) EXPECT_GT(llrs[0], 0.0) << "b0=1 should give LLR>0";
            else         EXPECT_LT(llrs[0], 0.0) << "b0=0 should give LLR<0";

            if (b1 == 1) EXPECT_GT(llrs[1], 0.0) << "b1=1 should give LLR>0";
            else         EXPECT_LT(llrs[1], 0.0) << "b1=0 should give LLR<0";
        }
    }
}

// Полный round-trip: modulate → demodulate → значения LLR = координаты символов.
TEST(QpskRoundTrip, ModulateDemodulate20Bits) {
    Bits bits(20);
    for (int i = 0; i < 20; ++i) bits[i] = i % 2;

    auto sym  = modulate(bits);
    auto llrs = demodulate(sym);

    ASSERT_EQ(llrs.size(), 20u);
    for (int i = 0; i < 20; ++i) {
        double expected = (bits[i] == 1) ? INV_SQRT2 : -INV_SQRT2;
        EXPECT_NEAR(llrs[i], expected, EPS) << "i=" << i;
    }
}
