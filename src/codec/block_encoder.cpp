#include "codec/block_encoder.hpp"
#include "codec/generator_matrix.hpp"

#include <stdexcept>

namespace pucch {

Bits encode(const Bits& data, int n_bits) {
    if (data.size() != static_cast<std::size_t>(n_bits)) {
        throw std::invalid_argument("data.size() does not match n_bits");
    }

    const int offset = col_offset(n_bits);

    Bits codeword(20, 0);
    for (int i = 0; i < 20; ++i) {
        uint8_t bit = 0;
        for (int j = 0; j < n_bits; ++j) {
            bit ^= G[i][offset + j] & data[j];
        }
        codeword[i] = bit;
    }
    return codeword;
}

} // namespace pucch
