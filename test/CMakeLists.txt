cmake_minimum_required(VERSION 3.16)

# all the test executables should link against gtest_main
link_libraries(GTest::Main)

add_executable(
    pdmath_test
    golden_search_test.cc
    helpers_test.cc
    line_search_test.cc
    math_functions_test.cc
    math_functors_test.cc
    norms_test.cc
)

include(GoogleTest)
# the official guidance is to use gtest_discover_tests, but do note that this
# results in ctest rerunning the test runner for each test case. this allows
# each test to be registered to ctest instead of treating the test runner as
# a single big test, but when run sequentially, results in a big slowdown. the
# test running script will by default use half the available threads to invoke
# the repeated test runner calls in parallel in an effort to speed things up.
gtest_discover_tests(pdmath_test)
# add_test(pdmath_test pdmath_test --gtest_color=yes)
