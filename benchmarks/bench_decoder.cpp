#include "codec/block_encoder.hpp"
#include "codec/soft_decoder.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>

using namespace pucch;
using Clock = std::chrono::high_resolution_clock;

struct BenchResult {
    long long   iterations;
    double      elapsed_sec;
    double      ops_per_sec;
};

// Запускает decode() на фиксированных LLR в течение duration_sec секунд.
static BenchResult run(int n_bits, double duration_sec) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(-3.0, 3.0);

    LLRs llrs(20);
    for (auto& x : llrs) x = dist(rng);

    // Прогрев: даём кешу таблицы построиться до начала замера.
    decode(llrs, n_bits);

    long long count   = 0;
    auto      start   = Clock::now();
    auto      deadline = start + std::chrono::duration_cast<Clock::duration>(
                             std::chrono::duration<double>(duration_sec));

    while (Clock::now() < deadline) {
        // volatile предотвращает выбрасывание вызова оптимизатором.
        [[maybe_unused]] volatile auto _ = decode(llrs, n_bits);
        ++count;
    }

    double elapsed = std::chrono::duration<double>(Clock::now() - start).count();
    return {count, elapsed, static_cast<double>(count) / elapsed};
}

int main() {
    constexpr double DURATION_SEC = 1.0;

    std::cout << "=== Soft decoder throughput benchmark ===\n";
    std::cout << "Measurement duration per n: " << DURATION_SEC << " s\n\n";
    std::cout << std::left
              << std::setw(6)  << "n"
              << std::setw(16) << "decode/sec"
              << std::setw(16) << "ns/decode"
              << "candidates\n";
    std::cout << std::string(54, '-') << "\n";

    for (int n : {2, 4, 6, 8, 11}) {
        auto r = run(n, DURATION_SEC);
        double ns_per_op = 1e9 / r.ops_per_sec;
        int    candidates = 1 << n;

        std::cout << std::left  << std::setw(6) << n
                  << std::right << std::setw(14) << std::fixed << std::setprecision(0)
                  << r.ops_per_sec << "  "
                  << std::setw(12) << std::setprecision(1) << ns_per_op << "  "
                  << candidates << "\n";
    }

    return 0;
}
