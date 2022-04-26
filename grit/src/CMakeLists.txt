add_library(GRIT
    utility.cpp
    trianglelist.cpp
    hypergraph.cpp
    gibbs_base.cpp
    generator.cpp

    observations-models/poisson_hypergraph.cpp
    observations-models/poisson_edgestrength.cpp

    hypergraph-models/independent_triangles.cpp
    hypergraph-models/independent_hyperedges.cpp
    hypergraph-models/edgestrength.cpp
    hypergraph-models/gilbert.cpp

    parameters-samplers/poisson_edgestrength.cpp
    parameters-samplers/poisson_gilbert.cpp
    parameters-samplers/poisson_independent_hyperedges.cpp
    parameters-samplers/poisson_independent_hyperedges_ordered.cpp

    proposers/twosteps_edges.cpp
    proposers/twosteps_triangle.cpp
    proposers/foursteps_hypergraph.cpp
    proposers/sixsteps_hypergraph.cpp

    proposers/edge-choosers/uniform_edge_chooser.cpp
    proposers/edge-choosers/weighted_chooser.cpp
    proposers/edge-choosers/weighted_two-layers_chooser.cpp
    proposers/edge-choosers/weighted_unique_chooser.cpp
    proposers/edge-choosers/sep-weighted_unique_chooser.cpp

    proposers/triangle-choosers/observations_by_pair_chooser.cpp
    proposers/triangle-choosers/random_chooser.cpp
    proposers/triangle-choosers/triplet_three_steps.cpp
    proposers/triangle-choosers/triplet_weighted.cpp
    proposers/triangle-choosers/uniform_triangle_chooser.cpp
)

set_target_properties(GRIT PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(GRIT PROPERTIES POSITION_INDEPENDENT_CODE TRUE)  # Required for pybind11 linking

target_link_libraries(GRIT ${CMAKE_SOURCE_DIR}/include/SamplableSet/src/build/libsamplableset.a)
target_link_libraries(GRIT ${Boost_LIBRARIES})
