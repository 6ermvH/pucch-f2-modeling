add_executable(pucch_f2_tests
    test_placeholder.cpp
    test_encoder.cpp
    test_decoder.cpp
    test_qpsk.cpp
    test_channel.cpp
)
target_link_libraries(pucch_f2_tests PRIVATE pucch_f2_lib GTest::gtest_main)
target_compile_options(pucch_f2_tests PRIVATE ${PUCCH_WARNINGS})

include(GoogleTest)
gtest_discover_tests(pucch_f2_tests)
