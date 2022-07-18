cmake_minimum_required(VERSION 3.16)

# all the test executables should link against gtest_main
link_libraries(GTest::Main)

add_executable(
    pdmath_test
    golden_search_test.cc
    helpers_test.cc
    line_search_test.cc
    norms_test.cc
)

include(GoogleTest)
gtest_discover_tests(pdmath_test)
