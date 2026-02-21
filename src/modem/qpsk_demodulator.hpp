#pragma once

#include "types.hpp"

namespace pucch {

// Тривиальный мягкий QPSK демодулятор.
// Для каждого принятого символа y:
//   LLR[2k]   = Re(y[k])   — мягкая метрика для b0
//   LLR[2k+1] = Im(y[k])   — мягкая метрика для b1
//
// Положительное LLR → бит=1 более вероятен.
// Возвращает вектор из 2·symbols.size() значений.
LLRs demodulate(const Symbols& symbols);

} // namespace pucch
