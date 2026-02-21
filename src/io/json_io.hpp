#pragma once

#include "simulation/bler_simulator.hpp"
#include "types.hpp"

#include <complex>
#include <string>

namespace pucch {

struct InputData {
    std::string mode;         // "coding" | "decoding" | "channel simulation"
    int         n_bits = 0;   // num_of_pucch_f2_bits ∈ {2,4,6,8,11}

    // coding
    Bits pucch_f2_bits;

    // decoding
    Symbols qpsk_symbols;

    // channel simulation
    double snr_db     = 0.0;
    int    iterations = 0;
};

// Читает и валидирует входной JSON файл.
// Бросает std::runtime_error при любой ошибке формата.
InputData parse_input(const std::string& filename);

// Записывают result.json для каждого режима.
void write_result_coding(const Symbols& symbols,
                         const std::string& filename = "result.json");

void write_result_decoding(int n_bits, const Bits& bits,
                           const std::string& filename = "result.json");

void write_result_simulation(int n_bits, const BlerResult& result,
                             const std::string& filename = "result.json");

// ── Утилиты (вынесены для тестирования) ──────────────────────────────────────

// Парсит строку вида "0.536+0.357j" или "-1.0-0.5j".
std::complex<double> parse_complex(const std::string& s);

// Форматирует комплексное число в строку "re+imj" / "re-imj".
std::string format_complex(std::complex<double> c);

} // namespace pucch
