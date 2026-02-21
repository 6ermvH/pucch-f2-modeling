#pragma once

#include "types.hpp"

namespace pucch {

// Кодирует n_bits бит данных в кодовое слово длиной 20 бит.
// n_bits должно быть одним из: 2, 4, 6, 8, 11.
// data.size() должно равняться n_bits.
Bits encode(const Bits& data, int n_bits);

} // namespace pucch
