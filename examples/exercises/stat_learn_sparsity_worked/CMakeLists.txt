cmake_minimum_required(VERSION 3.16)

# executables for the individual exercises
add_executable(ex4.7 ex4.7.cc)
target_link_libraries(ex4.7 PRIVATE Boost::headers Eigen3::Eigen)

# target_link_libraries(ex4.7 optimize)
# add_executable(ex4.9 ex4.9.cc)
