#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"
#include "GRIT/inference-models/phg.h"
#include "GRIT/inference-models/pes.h"
#include "GRIT/inference-models/per.h"

namespace py = pybind11;


// Extern modules
void defineAdditionalUtils(py::module &m);
void defineMetrics(py::module &m);
void defineModels(py::module &m);
void defineRandomHypergraphFunctions(py::module &m);
void defineRandomObservationsGeneration(py::module &m);


void seedRNG(size_t _seed) {
    GRIT::generator.seed(_seed);
}

PYBIND11_MODULE(pygrit, m) {
    defineAdditionalUtils(m);
    defineModels(m);
    defineMetrics(m);
    defineRandomHypergraphFunctions(m);
    defineRandomObservationsGeneration(m);

    m.def("seed", &seedRNG);

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
