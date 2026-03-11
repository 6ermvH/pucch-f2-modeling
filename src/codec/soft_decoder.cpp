#include "codec/soft_decoder.hpp"
#include "codec/block_encoder.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace pucch {

using CodeTable = std::vector<uint32_t>;

static CodeTable build_table(int n_bits) {
    const int num = 1 << n_bits;
    CodeTable table(num);

    for (int d = 0; d < num; ++d) {
        Bits data(n_bits);
        for (int j = 0; j < n_bits; ++j) data[j] = (d >> j) & 1;

        Bits cw = encode(data, n_bits);

        uint32_t packed = 0;
        for (int i = 0; i < 20; ++i) {
            if (cw[i]) packed |= (1u << i);
        }
        table[d] = packed;
    }
    return table;
}

static const CodeTable& get_table(int n_bits) {
    static std::unordered_map<int, CodeTable> cache;
    auto it = cache.find(n_bits);
    if (it != cache.end()) return it->second;
    return cache.emplace(n_bits, build_table(n_bits)).first->second;
}

Bits decode(const LLRs& llrs, int n_bits) {
    if (llrs.size() != 20) {
        throw std::invalid_argument("llrs.size() must be 20");
    }

    const CodeTable& table = get_table(n_bits);
    const int num = 1 << n_bits;

    int    best_d      = 0;
    double best_metric = -std::numeric_limits<double>::infinity();

    for (int d = 0; d < num; ++d) {
        uint32_t packed = table[d];
        double metric = 0.0;
        for (int i = 0; i < 20; ++i) {
            if ((packed >> i) & 1) metric += llrs[i];
        }
        if (metric > best_metric) {
            best_metric = metric;
            best_d      = d;
        }
    }

    Bits result(n_bits);
    for (int j = 0; j < n_bits; ++j) result[j] = (best_d >> j) & 1;
    return result;
}

}
