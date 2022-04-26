add_executable(observations observations.cpp)
add_executable(hypergraphs hypergraphs.cpp)

target_link_libraries(observations gtest gtest_main GRIT)
target_link_libraries(hypergraphs gtest gtest_main GRIT)

add_test(observations observations)
add_test(hypergraphs hypergraphs)
