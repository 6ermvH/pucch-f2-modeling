#include "io/json_io.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>

namespace pucch {

using json = nlohmann::json;

static const std::set<int> VALID_N = {2, 4, 6, 8, 11};

static void require(bool cond, const std::string& msg) {
    if (!cond) throw std::runtime_error(msg);
}

static json read_json_file(const std::string& filename) {
    std::ifstream f(filename);
    require(f.is_open(), "Cannot open file: " + filename);
    try {
        return json::parse(f);
    } catch (const json::parse_error& e) {
        throw std::runtime_error(std::string("JSON parse error: ") + e.what());
    }
}

static void write_json_file(const json& j, const std::string& filename) {
    std::ofstream f(filename);
    require(f.is_open(), "Cannot write file: " + filename);
    f << std::setw(2) << j << "\n";
}

static int get_n_bits(const json& j) {
    require(j.contains("num_of_pucch_f2_bits"),
            "Missing field: num_of_pucch_f2_bits");
    require(j["num_of_pucch_f2_bits"].is_number_integer(),
            "num_of_pucch_f2_bits must be an integer");
    int n = j["num_of_pucch_f2_bits"].get<int>();
    require(VALID_N.count(n) > 0,
            "num_of_pucch_f2_bits must be one of {2,4,6,8,11}, got: " + std::to_string(n));
    return n;
}

std::complex<double> parse_complex(const std::string& s) {
    require(!s.empty() && s.back() == 'j',
            "Invalid complex format (must end with 'j'): " + s);

    const std::string body = s.substr(0, s.size() - 1);

    std::size_t split = std::string::npos;
    for (std::size_t i = 1; i < body.size(); ++i) {
        char c = body[i];
        if ((c == '+' || c == '-') && body[i - 1] != 'e' && body[i - 1] != 'E') {
            split = i;
        }
    }

    require(split != std::string::npos,
            "Invalid complex format (no imaginary part): " + s);

    try {
        double re = std::stod(body.substr(0, split));
        double im = std::stod(body.substr(split));
        return {re, im};
    } catch (...) {
        throw std::runtime_error("Invalid complex format (bad number): " + s);
    }
}

std::string format_complex(std::complex<double> c) {
    std::ostringstream oss;
    oss << std::setprecision(10) << std::defaultfloat;
    oss << c.real();
    if (c.imag() >= 0.0) oss << "+" << c.imag() << "j";
    else                  oss << c.imag() << "j";
    return oss.str();
}

InputData parse_input(const std::string& filename) {
    const json j = read_json_file(filename);

    require(j.contains("mode"), "Missing field: mode");
    require(j["mode"].is_string(), "Field 'mode' must be a string");

    InputData in;
    in.mode = j["mode"].get<std::string>();

    if (in.mode == "coding") {
        in.n_bits = get_n_bits(j);

        require(j.contains("pucch_f2_bits"), "Missing field: pucch_f2_bits");
        require(j["pucch_f2_bits"].is_array(), "pucch_f2_bits must be an array");

        const auto& arr = j["pucch_f2_bits"];
        require(static_cast<int>(arr.size()) == in.n_bits,
                "pucch_f2_bits length must equal num_of_pucch_f2_bits (" +
                std::to_string(in.n_bits) + "), got: " + std::to_string(arr.size()));

        in.pucch_f2_bits.reserve(in.n_bits);
        for (std::size_t i = 0; i < arr.size(); ++i) {
            require(arr[i].is_number_integer(), "pucch_f2_bits elements must be integers");
            int b = arr[i].get<int>();
            require(b == 0 || b == 1,
                    "pucch_f2_bits elements must be 0 or 1, got: " + std::to_string(b));
            in.pucch_f2_bits.push_back(static_cast<uint8_t>(b));
        }

    } else if (in.mode == "decoding") {
        in.n_bits = get_n_bits(j);

        require(j.contains("qpsk_symbols"), "Missing field: qpsk_symbols");
        require(j["qpsk_symbols"].is_array(), "qpsk_symbols must be an array");

        const auto& arr = j["qpsk_symbols"];
        require(arr.size() == 10u,
                "qpsk_symbols must have exactly 10 elements (20-bit codeword), got: " +
                std::to_string(arr.size()));

        in.qpsk_symbols.reserve(10);
        for (std::size_t i = 0; i < arr.size(); ++i) {
            require(arr[i].is_string(),
                    "qpsk_symbols[" + std::to_string(i) + "] must be a string");
            in.qpsk_symbols.push_back(parse_complex(arr[i].get<std::string>()));
        }

    } else if (in.mode == "channel simulation") {
        in.n_bits = get_n_bits(j);

        require(j.contains("snr_db"), "Missing field: snr_db");
        require(j["snr_db"].is_number(), "snr_db must be a number");
        in.snr_db = j["snr_db"].get<double>();

        require(j.contains("iterations"), "Missing field: iterations");
        require(j["iterations"].is_number_integer(), "iterations must be an integer");
        in.iterations = j["iterations"].get<int>();
        require(in.iterations > 0,
                "iterations must be positive, got: " + std::to_string(in.iterations));

    } else {
        throw std::runtime_error(
            "Unknown mode: '" + in.mode +
            "'. Must be 'coding', 'decoding', or 'channel simulation'");
    }

    return in;
}

void write_result_coding(const Symbols& symbols, const std::string& filename) {
    json j;
    j["mode"] = "coding";
    j["qpsk_symbols"] = json::array();
    for (auto& s : symbols) j["qpsk_symbols"].push_back(format_complex(s));
    write_json_file(j, filename);
}

void write_result_decoding(int n_bits, const Bits& bits, const std::string& filename) {
    json j;
    j["mode"]                 = "decoding";
    j["num_of_pucch_f2_bits"] = n_bits;
    j["pucch_f2_bits"]        = bits;
    write_json_file(j, filename);
}

void write_result_simulation(int n_bits, const BlerResult& result,
                             const std::string& filename) {
    json j;
    j["mode"]                 = "channel simulation";
    j["num_of_pucch_f2_bits"] = n_bits;
    j["bler"]                 = result.bler;
    j["success"]              = result.success;
    j["failed"]               = result.failed;
    write_json_file(j, filename);
}

}
