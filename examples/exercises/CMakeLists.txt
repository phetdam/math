cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# executables for individual Statistical Learning with Sparsity exercises
add_executable(stat_learn_sparsity_4.7 stat_learn_sparsity_4.7.cc)
target_link_libraries(
    stat_learn_sparsity_4.7 PRIVATE
    pdmath::Boost pdmath::Eigen3 pdmath
)
add_test(NAME stat_learn_sparsity_4.7 COMMAND stat_learn_sparsity_4.7)

# TODO: incomplete
# add_executable(stat_learn_sparsity_4.9 stat_learn_sparsity_4.9.cc)
