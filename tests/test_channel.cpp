#include <gtest/gtest.h>
#include "channel/awgn_channel.hpp"
#include "simulation/bler_simulator.hpp"

#include <cmath>
#include <numeric>

using namespace pucch;

TEST(AwgnChannel, NoiseVarianceMatchesSNR) {
    constexpr double snr_db      = 10.0;
    constexpr double snr_linear  = 10.0;
    constexpr double sigma2      = 1.0 / (2.0 * snr_linear);
    constexpr int    N           = 50000;

    std::mt19937 rng(0);
    Symbols tx(N, {0.0, 0.0});
    Symbols rx = add_noise(tx, snr_db, rng);

    std::vector<double> samples;
    samples.reserve(2 * N);
    for (auto& s : rx) {
        samples.push_back(s.real());
        samples.push_back(s.imag());
    }

    double mean = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();
    double var  = 0.0;
    for (double x : samples) var += (x - mean) * (x - mean);
    var /= samples.size();

    EXPECT_NEAR(mean, 0.0,    0.02);
    EXPECT_NEAR(var,  sigma2, sigma2 * 0.05);
}

TEST(AwgnChannel, HighSNRLeavesSignalIntact) {
    std::mt19937 rng(1);
    Symbols tx = {{0.5, -0.3}, {-0.7, 0.9}};
    Symbols rx = add_noise(tx, 60.0, rng);

    for (std::size_t k = 0; k < tx.size(); ++k) {
        EXPECT_NEAR(rx[k].real(), tx[k].real(), 1e-2) << "k=" << k;
        EXPECT_NEAR(rx[k].imag(), tx[k].imag(), 1e-2) << "k=" << k;
    }
}

TEST(AwgnChannel, OutputSizeMatchesInput) {
    std::mt19937 rng(2);
    Symbols tx(10);
    EXPECT_EQ(add_noise(tx, 5.0, rng).size(), 10u);
}

TEST(BlerSimulator, HighSNRGivesZeroErrors) {
    for (int n : {2, 4, 6, 8, 11}) {
        auto r = simulate(n, 30.0, 500);
        EXPECT_EQ(r.failed, 0) << "n=" << n;
        EXPECT_EQ(r.success, 500) << "n=" << n;
        EXPECT_DOUBLE_EQ(r.bler, 0.0) << "n=" << n;
    }
}

TEST(BlerSimulator, LowSNRHasErrors) {
    auto r = simulate(11, -5.0, 1000);
    EXPECT_GT(r.failed, 0);
    EXPECT_GT(r.bler, 0.0);
}

TEST(BlerSimulator, SuccessPlusFailedEqualsIterations) {
    auto r = simulate(4, 5.0, 200);
    EXPECT_EQ(r.success + r.failed, 200);
    EXPECT_NEAR(r.bler, static_cast<double>(r.failed) / 200, 1e-12);
}

TEST(BlerSimulator, SameSeedGivesSameResult) {
    auto r1 = simulate(8, 3.0, 300, 42);
    auto r2 = simulate(8, 3.0, 300, 42);
    EXPECT_EQ(r1.failed, r2.failed);
}
