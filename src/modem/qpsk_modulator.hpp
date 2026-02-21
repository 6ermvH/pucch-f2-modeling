#pragma once

#include "types.hpp"

namespace pucch {

// Преобразует биты в QPSK символы: каждая пара бит (b0, b1) → один символ.
//
// Маппинг:
//   Re = (2·b0 − 1) / √2
//   Im = (2·b1 − 1) / √2
//
// Свойство: LLR демодулятора = координата символа (Re для b0, Im для b1).
// bits.size() должно быть чётным.
Symbols modulate(const Bits& bits);

} // namespace pucch
