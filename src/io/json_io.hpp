#pragma once

#include "simulation/bler_simulator.hpp"
#include "types.hpp"

#include <complex>
#include <string>

namespace pucch {

struct InputData {
    std::string mode;
    int         n_bits = 0;

    Bits pucch_f2_bits;

    Symbols qpsk_symbols;

    double snr_db     = 0.0;
    int    iterations = 0;
};

InputData parse_input(const std::string& filename);

void write_result_coding(const Symbols& symbols,
                         const std::string& filename = "result.json");

void write_result_decoding(int n_bits, const Bits& bits,
                           const std::string& filename = "result.json");

void write_result_simulation(int n_bits, const BlerResult& result,
                             const std::string& filename = "result.json");

std::complex<double> parse_complex(const std::string& s);

std::string format_complex(std::complex<double> c);

}
