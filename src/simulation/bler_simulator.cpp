#include "simulation/bler_simulator.hpp"

#include "channel/awgn_channel.hpp"
#include "codec/block_encoder.hpp"
#include "codec/soft_decoder.hpp"
#include "modem/qpsk_demodulator.hpp"
#include "modem/qpsk_modulator.hpp"

#include <random>

namespace pucch {

BlerResult simulate(int n_bits, double snr_db, int iterations, uint64_t seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> bit_dist(0, 1);

    int failed = 0;

    for (int iter = 0; iter < iterations; ++iter) {
        // Генерация случайных данных
        Bits data(n_bits);
        for (auto& b : data) b = static_cast<uint8_t>(bit_dist(rng));

        // Кодирование → модуляция → канал → демодуляция → декодирование
        Bits    codeword = encode(data, n_bits);
        Symbols tx       = modulate(codeword);
        Symbols rx       = add_noise(tx, snr_db, rng);
        LLRs    llrs     = demodulate(rx);
        Bits    decoded  = decode(llrs, n_bits);

        if (decoded != data) ++failed;
    }

    const int success = iterations - failed;
    const double bler = static_cast<double>(failed) / iterations;
    return {success, failed, bler};
}

} // namespace pucch
