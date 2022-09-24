add_executable(observations observations.cpp)
add_executable(structure structure.cpp)

target_link_libraries(observations gtest gtest_main GRIT)
target_link_libraries(structure gtest gtest_main GRIT)

add_test(observations observations)
add_test(structure structure)
