#include "modem/qpsk_demodulator.hpp"

namespace pucch {

LLRs demodulate(const Symbols& symbols) {
    LLRs llrs(2 * symbols.size());
    for (std::size_t k = 0; k < symbols.size(); ++k) {
        llrs[2 * k]     = symbols[k].real();
        llrs[2 * k + 1] = symbols[k].imag();
    }
    return llrs;
}

} // namespace pucch
