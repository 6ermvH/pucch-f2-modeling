add_executable(pucch_f2_bench
    bench_decoder.cpp
)
target_link_libraries(pucch_f2_bench PRIVATE pucch_f2_lib)
target_compile_options(pucch_f2_bench PRIVATE ${PUCCH_WARNINGS})
