#include <gtest/gtest.h>
#include "codec/block_encoder.hpp"
#include "codec/soft_decoder.hpp"

using namespace pucch;

// ── Вспомогательные утилиты ───────────────────────────────────────────────────

// Идеальные LLR по кодовому слову: +100 где бит=1, -100 где бит=0.
static LLRs perfect_llrs(const Bits& codeword) {
    LLRs llrs(20);
    for (int i = 0; i < 20; ++i) llrs[i] = codeword[i] ? 100.0 : -100.0;
    return llrs;
}

// ── Тесты ─────────────────────────────────────────────────────────────────────

// Encode → идеальные LLR → decode должен вернуть исходные данные.
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

// Все 2^n паттернов для n=4 (16 вариантов — полный перебор).
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

// Выборочные паттерны для n=11.
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

// Размер результата декодирования всегда равен n_bits.
TEST(SoftDecoder, OutputSizeEqualsNBits) {
    for (int n : {2, 4, 6, 8, 11}) {
        LLRs llrs(20, 1.0);
        EXPECT_EQ(decode(llrs, n).size(), static_cast<std::size_t>(n));
    }
}

// Неверный размер LLR → исключение.
TEST(SoftDecoder, WrongLLRSizeThrows) {
    EXPECT_THROW(decode(LLRs(19, 0.0), 4), std::invalid_argument);
    EXPECT_THROW(decode(LLRs(21, 0.0), 4), std::invalid_argument);
}

// Знак LLR: кандидат с большей метрикой побеждает.
// Подаём LLR, чуть сдвинутые в сторону известного кодового слова.
TEST(SoftDecoder, PicksHigherMetricCandidate) {
    constexpr int n = 2;
    // data=[1,0], data=[0,1] — два разных кодовых слова
    Bits cw_10 = encode({1, 0}, n);
    Bits cw_01 = encode({0, 1}, n);

    // LLR точно совпадает с кодовым словом [1,0]
    EXPECT_EQ(decode(perfect_llrs(cw_10), n), (Bits{1, 0}));
    // LLR точно совпадает с кодовым словом [0,1]
    EXPECT_EQ(decode(perfect_llrs(cw_01), n), (Bits{0, 1}));
}
