#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"
#include "GRIT/inference-models/phg.h"
#include "GRIT/inference-models/pes.h"
#include "GRIT/inference-models/per.h"


namespace py = pybind11;


void defineModels(py::module &m) {

    py::class_<PHG> (m, "PHG")
        .def(py::init<size_t, double, size_t, size_t,
                      double, double, double,
                      const std::vector<double>&, const std::vector<double>&>(),
                py::arg("window_size"), py::arg("tolerance"), py::arg("mh_minit"), py::arg("mh_maxit"),
                py::arg("eta"), py::arg("chi_0"), py::arg("chi_1"),
                py::arg("model_hyperparameters"), py::arg("move_probabilities")
            )
        .def("set_hyperparameters", &PHG::setHyperparameters, py::arg("hyperparameters"))
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
        .def("set_hyperparameters", &PES::setHyperparameters, py::arg("hyperparameters"))
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
        .def("set_hyperparameters", &PER::setHyperparameters, py::arg("hyperparameters"))
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
