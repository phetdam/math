cmake_minimum_required(VERSION 3.16)

# all the test executables should link against gtest_main
link_libraries(GTest::Main)

add_executable(pdmath_optimize_test golden_search_test.cc)

include(GoogleTest)
gtest_discover_tests(pdmath_optimize_test)
