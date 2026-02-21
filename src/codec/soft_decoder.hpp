#pragma once

#include "types.hpp"

namespace pucch {

// Мягкое декодирование методом перебора всех кодовых слов:
//   d̂ = argmax_d ( c(d) · llrs )
//
// llrs.size() должно быть равно 20.
// n_bits должно быть одним из: 2, 4, 6, 8, 11.
//
// Кодовые слова для всех 2^n_bits кандидатов предвычисляются при первом
// вызове для данного n_bits и кешируются на всё время жизни программы.
Bits decode(const LLRs& llrs, int n_bits);

} // namespace pucch
