#include <gtest/gtest.h>

#include "channel/awgn_channel.hpp"
#include "codec/block_encoder.hpp"
#include "codec/soft_decoder.hpp"
#include "io/json_io.hpp"
#include "modem/qpsk_demodulator.hpp"
#include "modem/qpsk_modulator.hpp"
#include "simulation/bler_simulator.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <random>

using namespace pucch;

// ── parse_complex / format_complex ────────────────────────────────────────────

TEST(JsonIo, ParseComplexPositiveIm) {
    auto c = parse_complex("0.536+0.357j");
    EXPECT_NEAR(c.real(),  0.536, 1e-9);
    EXPECT_NEAR(c.imag(),  0.357, 1e-9);
}

TEST(JsonIo, ParseComplexNegativeIm) {
    auto c = parse_complex("0.536-0.357j");
    EXPECT_NEAR(c.real(),  0.536, 1e-9);
    EXPECT_NEAR(c.imag(), -0.357, 1e-9);
}

TEST(JsonIo, ParseComplexNegativeRe) {
    auto c = parse_complex("-1.0+0.5j");
    EXPECT_NEAR(c.real(), -1.0, 1e-9);
    EXPECT_NEAR(c.imag(),  0.5, 1e-9);
}

TEST(JsonIo, ParseComplexBothNegative) {
    auto c = parse_complex("-0.707-0.707j");
    EXPECT_NEAR(c.real(), -0.707, 1e-9);
    EXPECT_NEAR(c.imag(), -0.707, 1e-9);
}

TEST(JsonIo, ParseComplexRoundTrip) {
    std::complex<double> orig{1.23456789, -9.87654321};
    auto s    = format_complex(orig);
    auto back = parse_complex(s);
    EXPECT_NEAR(back.real(), orig.real(), 1e-8);
    EXPECT_NEAR(back.imag(), orig.imag(), 1e-8);
}

TEST(JsonIo, ParseComplexInvalidThrows) {
    EXPECT_THROW(parse_complex("1.0+0.5"),  std::runtime_error); // нет 'j'
    EXPECT_THROW(parse_complex("1.0j"),     std::runtime_error); // нет разделителя
    EXPECT_THROW(parse_complex(""),         std::runtime_error);
}

TEST(JsonIo, FormatComplexPositiveIm) {
    std::string s = format_complex({0.5, 0.3});
    EXPECT_NE(s.find('+'), std::string::npos);
    EXPECT_EQ(s.back(), 'j');
}

TEST(JsonIo, FormatComplexNegativeIm) {
    std::string s = format_complex({0.5, -0.3});
    EXPECT_EQ(s.back(), 'j');
    // знак минус должен быть где-то после первого символа
    EXPECT_NE(s.rfind('-'), std::string::npos);
}

// ── Вспомогательная функция: write/read JSON через tmp-файл ───────────────────

static const std::string TMP_IN  = "/tmp/pucch_test_input.json";
static const std::string TMP_OUT = "/tmp/pucch_test_result.json";

static void write_tmp(const std::string& content) {
    std::ofstream f(TMP_IN);
    f << content;
}

// ── parse_input: корректные входные данные ────────────────────────────────────

TEST(JsonIo, ParseCodingMode) {
    write_tmp(R"({"mode":"coding","num_of_pucch_f2_bits":4,"pucch_f2_bits":[1,0,1,1]})");
    auto in = parse_input(TMP_IN);
    EXPECT_EQ(in.mode,   "coding");
    EXPECT_EQ(in.n_bits, 4);
    EXPECT_EQ(in.pucch_f2_bits, (Bits{1,0,1,1}));
}

TEST(JsonIo, ParseDecodingMode) {
    // 10 символов
    std::string symbols = "";
    for (int i = 0; i < 10; ++i) {
        if (i > 0) symbols += ",";
        symbols += "\"0.707+0.707j\"";
    }
    write_tmp(R"({"mode":"decoding","num_of_pucch_f2_bits":2,"qpsk_symbols":[)" + symbols + "]}");
    auto in = parse_input(TMP_IN);
    EXPECT_EQ(in.mode,   "decoding");
    EXPECT_EQ(in.n_bits, 2);
    EXPECT_EQ(in.qpsk_symbols.size(), 10u);
}

TEST(JsonIo, ParseChannelSimulationMode) {
    write_tmp(R"({"mode":"channel simulation","num_of_pucch_f2_bits":11,"snr_db":5.0,"iterations":100})");
    auto in = parse_input(TMP_IN);
    EXPECT_EQ(in.mode,       "channel simulation");
    EXPECT_EQ(in.n_bits,     11);
    EXPECT_DOUBLE_EQ(in.snr_db, 5.0);
    EXPECT_EQ(in.iterations, 100);
}

// ── parse_input: ошибочные входные данные ────────────────────────────────────

TEST(JsonIo, ErrorOnUnknownMode) {
    write_tmp(R"({"mode":"unknown","num_of_pucch_f2_bits":2})");
    EXPECT_THROW(parse_input(TMP_IN), std::runtime_error);
}

TEST(JsonIo, ErrorOnMissingMode) {
    write_tmp(R"({"num_of_pucch_f2_bits":2,"pucch_f2_bits":[0,1]})");
    EXPECT_THROW(parse_input(TMP_IN), std::runtime_error);
}

TEST(JsonIo, ErrorOnInvalidNBits) {
    write_tmp(R"({"mode":"coding","num_of_pucch_f2_bits":3,"pucch_f2_bits":[0,1,0]})");
    EXPECT_THROW(parse_input(TMP_IN), std::runtime_error);
}

TEST(JsonIo, ErrorOnWrongBitsArrayLength) {
    write_tmp(R"({"mode":"coding","num_of_pucch_f2_bits":4,"pucch_f2_bits":[0,1]})");
    EXPECT_THROW(parse_input(TMP_IN), std::runtime_error);
}

TEST(JsonIo, ErrorOnNonBinaryBit) {
    write_tmp(R"({"mode":"coding","num_of_pucch_f2_bits":2,"pucch_f2_bits":[0,5]})");
    EXPECT_THROW(parse_input(TMP_IN), std::runtime_error);
}

TEST(JsonIo, ErrorOnMissingSnrDb) {
    write_tmp(R"({"mode":"channel simulation","num_of_pucch_f2_bits":2,"iterations":100})");
    EXPECT_THROW(parse_input(TMP_IN), std::runtime_error);
}

TEST(JsonIo, ErrorOnZeroIterations) {
    write_tmp(R"({"mode":"channel simulation","num_of_pucch_f2_bits":2,"snr_db":5.0,"iterations":0})");
    EXPECT_THROW(parse_input(TMP_IN), std::runtime_error);
}

// ── Полный пайплайн ───────────────────────────────────────────────────────────

// При высоком SNR полный пайплайн восстанавливает исходные биты.
TEST(Integration, FullPipelineHighSNR) {
    for (int n : {2, 4, 6, 8, 11}) {
        Bits data(n);
        for (int j = 0; j < n; ++j) data[j] = j % 2;

        Bits    cw      = encode(data, n);
        Symbols tx      = modulate(cw);

        // Имитируем канал с SNR=30 dB
        std::mt19937 rng(0);
        Symbols rx  = pucch::add_noise(tx, 30.0, rng);
        LLRs    llrs = demodulate(rx);
        Bits    got  = decode(llrs, n);

        EXPECT_EQ(got, data) << "n=" << n;
    }
}

// Coding mode: write_result_coding → parse_complex round-trip.
TEST(Integration, CodingResultParseable) {
    Bits cw(20, 0);
    for (int i = 0; i < 20; i += 2) cw[i] = 1;
    Symbols sym = modulate(cw);

    write_result_coding(sym, TMP_OUT);

    // Читаем обратно и проверяем
    std::ifstream f(TMP_OUT);
    ASSERT_TRUE(f.is_open());
    auto j = nlohmann::json::parse(f);
    EXPECT_EQ(j["mode"], "coding");
    ASSERT_EQ(j["qpsk_symbols"].size(), 10u);

    for (std::size_t k = 0; k < 10; ++k) {
        auto c = parse_complex(j["qpsk_symbols"][k].get<std::string>());
        EXPECT_NEAR(c.real(), sym[k].real(), 1e-8);
        EXPECT_NEAR(c.imag(), sym[k].imag(), 1e-8);
    }
}

// Decoding mode: write → read → verify.
TEST(Integration, DecodingResultRoundTrip) {
    Bits bits = {1, 0, 1, 1, 0, 0, 1, 1};
    write_result_decoding(8, bits, TMP_OUT);

    std::ifstream f(TMP_OUT);
    auto j = nlohmann::json::parse(f);
    EXPECT_EQ(j["mode"],                 "decoding");
    EXPECT_EQ(j["num_of_pucch_f2_bits"], 8);
    EXPECT_EQ(j["pucch_f2_bits"],        bits);
}

// Simulation mode: write → read → verify fields.
TEST(Integration, SimulationResultFields) {
    BlerResult r{900, 100, 0.1};
    write_result_simulation(11, r, TMP_OUT);

    std::ifstream f(TMP_OUT);
    auto j = nlohmann::json::parse(f);
    EXPECT_EQ(j["mode"],                 "channel simulation");
    EXPECT_EQ(j["num_of_pucch_f2_bits"], 11);
    EXPECT_NEAR(j["bler"].get<double>(), 0.1, 1e-12);
    EXPECT_EQ(j["success"],              900);
    EXPECT_EQ(j["failed"],               100);
}
