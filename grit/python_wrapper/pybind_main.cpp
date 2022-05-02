#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"
#include "GRIT/inference-models/phg.h"
#include "GRIT/inference-models/pes.h"
#include "GRIT/inference-models/per.h"

namespace py = pybind11;

#include "additional_utility.hpp"
#include "metrics.hpp"
#include "observations_generation.hpp"
#include "hypergraph_generation.hpp"


void seedRNG(size_t _seed) {
    GRIT::generator.seed(_seed);
}

PYBIND11_MODULE(pygrit, m)
{
    py::class_<GRIT::Triplet> (m, "Triplet")
        .def(py::init<>())
        .def_readwrite("i", &GRIT::Triplet::i)
        .def_readwrite("j", &GRIT::Triplet::j)
        .def_readwrite("k", &GRIT::Triplet::k)
        .def("__eq__", &GRIT::Triplet::operator==)
        .def("__ne__", &GRIT::Triplet::operator!=);

    py::class_<GRIT::TriangleList> (m, "TriangleList")
        .def(py::init<size_t>())
        .def("get_triangle_number", &GRIT::TriangleList::getTriangleNumber)
        .def_static("load_from_binary", &GRIT::TriangleList::loadFromBinary);

    py::class_<GRIT::Hypergraph> (m, "Hypergraph")
        .def(py::init<size_t>())
        .def("add_edge", &GRIT::Hypergraph::addEdge)
        .def("add_triangle", [](GRIT::Hypergraph& self, size_t i, size_t j, size_t k) { self.addTriangle({i, j, k}); })
        .def("is_edge", &GRIT::Hypergraph::isEdge)
        .def("resize", &GRIT::Hypergraph::resize)
        .def("get_size", &GRIT::Hypergraph::getSize)
        .def("get_edge_number", &GRIT::Hypergraph::getEdgeNumber)
        .def("get_edge_multiplicity", &GRIT::Hypergraph::getEdgeMultiplicity)
        .def("get_edges_from", &GRIT::Hypergraph::getEdgesFrom)
        .def("get_highest_order_hyperedge_with", &GRIT::Hypergraph::getHighestOrderHyperedgeWith)
        .def("get_full_triangle_list", &GRIT::Hypergraph::getFullTriangleList)
        .def("get_triangle_list", &GRIT::Hypergraph::getTriangles)
        .def("get_triangle_number", &GRIT::Hypergraph::getTriangleNumber)
        .def("get_triangles_from", &GRIT::Hypergraph::getTrianglesFrom)
        .def("load_from_binary", &GRIT::Hypergraph::loadFromBinary)
        .def("write_to_binary", &GRIT::Hypergraph::writeToBinary)
        .def("get_copy", [](const GRIT::Hypergraph& self){ return GRIT::Hypergraph(self); });


    py::class_<PHG> (m, "PHG")
        .def(py::init<size_t, double, size_t, size_t,
                      double, double, double,
                      const std::vector<double>&, const std::vector<double>&>(),
                py::arg("window_size"), py::arg("tolerance"), py::arg("mh_minit"), py::arg("mh_maxit"),
                py::arg("eta"), py::arg("chi_0"), py::arg("chi_1"),
                py::arg("model_hyperparameters"), py::arg("move_probabilities")
            )
        .def("sample", &PHG::sample,
                py::arg("sample_size"), py::arg("burnin"), py::arg("chain"),
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations"), py::arg("output_directory")
            )
        .def("sample_hypergraph_chain", &PHG::sampleHypergraphs,
                py::arg("mh_steps"), py::arg("points"), py::arg("gibbs_iterations"),
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations"), py::arg("output_directory")
            )
        .def("get_loglikelihood", &PHG::getLogLikelihood,
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations")
            )
        .def("get_pairwise_observations_probabilities", &PHG::getPairwiseObservationsProbabilities,
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations"))
        .def("generate_observations", &PHG::generateObservations,
                py::arg("hypergraph"), py::arg("parameters")
            );

    py::class_<PES> (m, "PES")
        .def(py::init<size_t, double, size_t, size_t,
                      double,
                      const std::vector<double>&, const std::vector<double>&>(),
                py::arg("window_size"), py::arg("tolerance"), py::arg("mh_minit"), py::arg("mh_maxit"),
                py::arg("eta"),
                py::arg("model_hyperparameters"), py::arg("move_probabilities")
            )
        .def("sample_hypergraph_chain", &PES::sampleHypergraphs,
                py::arg("mh_steps"), py::arg("points"), py::arg("gibbs_iterations"),
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations"), py::arg("output_directory")
            )
        .def("sample", &PES::sample,
                py::arg("sample_size"), py::arg("burnin"), py::arg("chain"),
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations"), py::arg("output_directory")
            )
        .def("get_loglikelihood", &PES::getLogLikelihood,
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations")
            )
        .def("get_pairwise_observations_probabilities", &PES::getPairwiseObservationsProbabilities,
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations"))
        .def("generate_observations", &PES::generateObservations,
                py::arg("hypergraph"), py::arg("parameters")
            );

    py::class_<PER> (m, "PER")
        .def(py::init<size_t, double, size_t, size_t,
                      double,
                      const std::vector<double>&, const std::vector<double>&>(),
                py::arg("window_size"), py::arg("tolerance"), py::arg("mh_minit"), py::arg("mh_maxit"),
                py::arg("eta"),
                py::arg("model_hyperparameters"), py::arg("move_probabilities")
            )
        .def("sample_hypergraph_chain", &PER::sampleHypergraphs,
                py::arg("mh_steps"), py::arg("points"), py::arg("gibbs_iterations"),
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations"), py::arg("output_directory")
            )
        .def("sample", &PER::sample,
                py::arg("sample_size"), py::arg("burnin"), py::arg("chain"),
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations"), py::arg("output_directory")
            )
        .def("get_loglikelihood", &PER::getLogLikelihood,
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations")
            )
        .def("get_pairwise_observations_probabilities", &PER::getPairwiseObservationsProbabilities,
                py::arg("hypergraph"), py::arg("parameters"), py::arg("observations"))
        .def("generate_observations", &PER::generateObservations,
                py::arg("hypergraph"), py::arg("parameters")
            );

    m.def("seed", &seedRNG);
    m.def("generate_poisson_observations", &generatePoissonObservations);
    m.def("generate_independent_hyperedges_hypergraph", &generateIndependentHyperedgesHypergraph);
    m.def("generate_independent_layered_edges_hypergraph", &generateIndependentLayeredEdges);
    m.def("generate_sbm_hypergraph", &generateSBMHypergraph);
    m.def("generate_miller_cm_hypergraph_geometric", &generateMillerConfigurationModelHypergraphWithGeometricDistribution);
    m.def("generate_beta_model_hypergraph_normal", &generateBetaModelHypergraphWithNormalDistributions);
    m.def("generate_independent_hyperedges_only_cycles", &generateIndependentHyperedgeOnlyCycles);
    m.def("generate_independent_hyperedges_no_cycles", &generateIndependentHyperedgeWithoutCycles);

    m.def("get_decreasing_ordered_pairs", &getDecreasingOrderedPairs);

    m.def("remove_disconnected_vertices", &removeDisconnectedVertices);
    m.def("project_hypergraph_on_multigraph", &projectHypergraphOnMultigraph);
    m.def("project_hypergraph_on_graph", &projectHypergraphOnGraph);
    m.def("generate_hypergraph_from_adjacency", &generateHypergraphFromAdjacencyMatrix);
    m.def("get_average_hypergraph", &getAverageHypergraph);
    m.def("get_average_hypergraph_edgestrength", &getAverageHypergraphEdgeStrength);

    m.def("count_edges_in_triangles", &countEdgesInTriangles);
    m.def("get_confusion_matrix", &getConfusionMatrix, py::arg("groundtruth"), py::arg("average edge types"), py::arg("with_correlation")=false, py::arg("edge_types_swapped")=false);
    m.def("get_sum_residuals_of_types", &getSumOfResidualsOfTypes, py::arg("edge_types"), py::arg("observations1"), py::arg("observations2"));
    m.def("get_sum_absolute_residuals_of_types", &getSumOfAbsoluteResidualsOfTypes, py::arg("edge_types"), py::arg("observations1"), py::arg("observations2"));

    m.def("get_edge_hamming_distance", &getEdgeHammingDistance);
    m.def("get_global_hamming_distance", &getGlobalHammingDistance);

    m.def("sample_from_beta", &GRIT::drawFromBeta);
    m.def("sample_from_linear", &GRIT::drawFromLinearDistribution);
    m.def("sample_from_lower_truncgamma_its", &GRIT::drawFromLowerTruncatedGammaITS);
    m.def("sample_from_shifted_geometric", &GRIT::drawFromShiftedGeometricDistribution);
    m.def("sample_from_truncgamma_gamma_rs", &GRIT::drawTruncatedGammaWithGammaRS);
    m.def("sample_from_truncgamma_its", &GRIT::drawFromTruncatedGammaITS);
    m.def("sample_from_truncgamma_uniform_rs", &GRIT::drawTruncatedGammaWithUniformRS);
    m.def("sample_from_truncgamma_linear_rs", &GRIT::drawTruncatedGammaWithLinearRS);
    m.def("sample_from_upper_truncgamma_its", &GRIT::drawFromUpperTruncatedGammaITS);
}
