#pragma once

#include <complex>
#include <cstdint>
#include <vector>

namespace pucch {

using Bits    = std::vector<uint8_t>;            // элементы: 0 или 1
using LLRs    = std::vector<double>;             // мягкие метрики
using Symbols = std::vector<std::complex<double>>; // QPSK символы

} // namespace pucch
