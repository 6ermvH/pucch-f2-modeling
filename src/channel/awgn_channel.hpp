#pragma once

#include "types.hpp"

#include <random>

namespace pucch {

// Добавляет аддитивный белый гауссовский шум к QPSK символам.
//
// При единичной энергии символа (Es = 1) и SNR в дБ:
//   snr_linear = 10^(snr_db / 10)
//   σ = √(1 / (2 · snr_linear))  — стандартное отклонение на каждую координату
//
// Возвращает y = s + n, где n_re, n_im ~ N(0, σ²) независимо.
Symbols add_noise(const Symbols& tx, double snr_db, std::mt19937& rng);

} // namespace pucch
