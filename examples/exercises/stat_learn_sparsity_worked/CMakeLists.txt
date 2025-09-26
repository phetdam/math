cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# executables for individual Statistical Learning with Sparsity exercises
add_executable(stat_learn_sparsity_4.7 ex4.7.cc)
target_link_libraries(stat_learn_sparsity_4.7 PRIVATE Boost::headers Eigen3::Eigen)
add_test(NAME stat_learn_sparsity_4.7 COMMAND stat_learn_sparsity_4.7)

# target_link_libraries(ex4.7 optimize)
# add_executable(ex4.9 ex4.9.cc)
