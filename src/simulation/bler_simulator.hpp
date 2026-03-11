#pragma once

#include <cstdint>

namespace pucch {

struct BlerResult {
    int    success;
    int    failed;
    double bler;
};

BlerResult simulate(int n_bits, double snr_db, int iterations, uint64_t seed = 42);

}
