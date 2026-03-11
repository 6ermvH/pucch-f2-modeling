#pragma once

#include "types.hpp"

#include <random>

namespace pucch {

Symbols add_noise(const Symbols& tx, double snr_db, std::mt19937& rng);

}
