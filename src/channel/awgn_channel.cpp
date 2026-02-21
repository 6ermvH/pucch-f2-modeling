#include "channel/awgn_channel.hpp"

#include <cmath>

namespace pucch {

Symbols add_noise(const Symbols& tx, double snr_db, std::mt19937& rng) {
    const double snr_linear = std::pow(10.0, snr_db / 10.0);
    const double sigma      = std::sqrt(1.0 / (2.0 * snr_linear));

    std::normal_distribution<double> noise(0.0, sigma);

    Symbols rx(tx.size());
    for (std::size_t k = 0; k < tx.size(); ++k) {
        rx[k] = {tx[k].real() + noise(rng), tx[k].imag() + noise(rng)};
    }
    return rx;
}

} // namespace pucch
