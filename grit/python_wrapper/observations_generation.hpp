#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"

#include <pybind11/numpy.h>


py::array_t<size_t> generatePoissonObservations(const GRIT::Hypergraph& hypergraph, double mu0, double mu1, double mu2, bool withTriangles){
    size_t n = hypergraph.getSize();

    py::array_t<size_t> observations({n, n});
    py::buffer_info observationsBuffer = observations.request();
    size_t *observationsPtr = (size_t *) observationsBuffer.ptr;

    std::poisson_distribution<size_t> distribution[3] = {
                            std::poisson_distribution<size_t>(mu0),
                            std::poisson_distribution<size_t>(mu1),
                            std::poisson_distribution<size_t>(mu2)};

    size_t hyperedgeType, observationsElement;
    for (size_t i=0; i<n; i++){
        observationsPtr[i*n+i] = 0; // initialize diagonal to 0
        for (size_t j=i+1; j<n; j++){
            if (withTriangles)
                hyperedgeType = hypergraph.getHighestOrderHyperedgeWith(i, j);
            else
                hyperedgeType = hypergraph.getEdgeMultiplicity(i, j);
            if (hyperedgeType > 2)
                hyperedgeType = 2;

            observationsElement = distribution[hyperedgeType](GRIT::generator);
            observationsPtr[i*n+j] = observationsElement;
            observationsPtr[j*n+i] = observationsElement;
        }
    }
    return observations;
}
