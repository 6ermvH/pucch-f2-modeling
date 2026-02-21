#include "modem/qpsk_modulator.hpp"

#include <cmath>
#include <stdexcept>

namespace pucch {

Symbols modulate(const Bits& bits) {
    if (bits.size() % 2 != 0) {
        throw std::invalid_argument("bits.size() must be even");
    }

    const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    Symbols symbols(bits.size() / 2);

    for (std::size_t k = 0; k < symbols.size(); ++k) {
        double re = (2.0 * bits[2 * k]     - 1.0) * inv_sqrt2;
        double im = (2.0 * bits[2 * k + 1] - 1.0) * inv_sqrt2;
        symbols[k] = {re, im};
    }
    return symbols;
}

} // namespace pucch
