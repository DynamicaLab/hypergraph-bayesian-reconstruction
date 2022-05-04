add_executable(TriangleChooser trianglechoosers.cpp)
add_executable(EdgeChooser edgechoosers.cpp)
#add_executable(EdgeProposal edgeproposer.cpp)
add_executable(HypergraphProposal hypergraph_proposers.cpp)

target_link_libraries(TriangleChooser gtest gtest_main GRIT)
target_link_libraries(EdgeChooser gtest gtest_main GRIT)
#target_link_libraries(EdgeProposal gtest gtest_main GRIT)
target_link_libraries(HypergraphProposal gtest gtest_main GRIT)

add_test(TriangleChooser TriangleChooser)
add_test(EdgeChooser TriangleChooser)
#add_test(EdgeProposal EdgeProposal)
add_test(HypergraphProposal EdgeProposal)
