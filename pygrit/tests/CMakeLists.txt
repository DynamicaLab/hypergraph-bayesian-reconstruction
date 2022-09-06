add_subdirectory(proposers)
add_subdirectory(models)


add_executable(TriangleList trianglelist.cpp)
add_executable(Hypergraph hypergraph.cpp)
add_executable(GibbsBase gibbs_base.cpp)

target_link_libraries(TriangleList gtest gtest_main GRIT)
target_link_libraries(Hypergraph gtest gtest_main GRIT)
target_link_libraries(GibbsBase gtest gtest_main GRIT)

add_test(TriangleList TriangleList)
add_test(Hypergraph Hypergraph)
add_test(GibbsBase GibbsBase)
