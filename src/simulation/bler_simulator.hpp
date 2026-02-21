#pragma once

#include <cstdint>

namespace pucch {

struct BlerResult {
    int    success;  // число блоков без ошибок
    int    failed;   // число блоков с ошибкой декодирования
    double bler;     // = failed / (success + failed)
};

// Симулирует передачу случайных блоков через канал АБГШ и возвращает BLER.
//
// Одна итерация:
//   случайные данные → encode → modulate → add_noise → demodulate → decode
//   → сравнение с исходными: если хотя бы один бит не совпал — блок ошибочный.
//
// seed — начальное значение генератора для воспроизводимости результатов.
BlerResult simulate(int n_bits, double snr_db, int iterations, uint64_t seed = 42);

} // namespace pucch
