#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"
#include "GRIT/inference-models/phg.h"
#include "GRIT/inference-models/pes.h"
#include "GRIT/inference-models/per.h"


namespace py = pybind11;


void defineModels(py::module &m) {
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
}
