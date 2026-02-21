cmake_minimum_required(VERSION 3.16...3.27)
project(pucch_f2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Dependencies ──────────────────────────────────────────────────────────────

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST          OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ── Compiler warnings ─────────────────────────────────────────────────────────

set(PUCCH_WARNINGS -Wall -Wextra -Wpedantic)

# ── Core library ──────────────────────────────────────────────────────────────

add_library(pucch_f2_lib STATIC
    src/codec/block_encoder.cpp
    src/codec/soft_decoder.cpp
    src/modem/qpsk_modulator.cpp
    src/modem/qpsk_demodulator.cpp
    src/channel/awgn_channel.cpp
    src/simulation/bler_simulator.cpp
    src/io/json_io.cpp
)
target_include_directories(pucch_f2_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pucch_f2_lib PUBLIC nlohmann_json::nlohmann_json)
target_compile_options(pucch_f2_lib PRIVATE ${PUCCH_WARNINGS})

# ── Main executable ───────────────────────────────────────────────────────────

add_executable(pucch_f2 src/main.cpp)
target_link_libraries(pucch_f2 PRIVATE pucch_f2_lib)
target_compile_options(pucch_f2 PRIVATE ${PUCCH_WARNINGS})

# ── Tests and benchmarks ──────────────────────────────────────────────────────

enable_testing()
add_subdirectory(tests)
add_subdirectory(benchmarks)
