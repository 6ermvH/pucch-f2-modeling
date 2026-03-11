#include "codec/block_encoder.hpp"
#include "codec/soft_decoder.hpp"
#include "io/json_io.hpp"
#include "modem/qpsk_demodulator.hpp"
#include "modem/qpsk_modulator.hpp"
#include "simulation/bler_simulator.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: pucch_f2 <input.json>\n";
        return 1;
    }

    try {
        const pucch::InputData in = pucch::parse_input(argv[1]);

        if (in.mode == "coding") {
            pucch::Bits    codeword = pucch::encode(in.pucch_f2_bits, in.n_bits);
            pucch::Symbols symbols  = pucch::modulate(codeword);
            pucch::write_result_coding(symbols);

        } else if (in.mode == "decoding") {
            pucch::LLRs llrs = pucch::demodulate(in.qpsk_symbols);
            pucch::Bits bits = pucch::decode(llrs, in.n_bits);
            pucch::write_result_decoding(in.n_bits, bits);

        } else {
            pucch::BlerResult result =
                pucch::simulate(in.n_bits, in.snr_db, in.iterations);
            pucch::write_result_simulation(in.n_bits, result);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
