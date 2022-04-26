#ifndef GRIT_POISSON_EDGETYPES_TRUNCATEDGAMMA_H
#define GRIT_POISSON_EDGETYPES_TRUNCATEDGAMMA_H


#include <stdexcept>

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"


namespace GRIT{

class PoissonEdgeStrengthParametersSampler {
    const Hypergraph& hypergraph;
    const Observations& observations;
    Parameters& parameters;
    const Parameters& hyperParameters;

    size_t nchoose2;

    public:
        PoissonEdgeStrengthParametersSampler(const Hypergraph& hypergraph, const Observations& observations, Parameters& parameters, const Parameters& hyperParameters):
                hypergraph(hypergraph), observations(observations), parameters(parameters), hyperParameters(hyperParameters)
        {
            nchoose2 = hypergraph.getSize()*(hypergraph.getSize()-1)/2;
            if (hyperParameters.size() != 10)
                throw std::logic_error("Parameter sampler: incorrect number of hyperparameters. "
                        "There are " + std::to_string(hyperParameters.size()) + " hyperparameters instead of 10");
        };
        void sample();

    private:
        std::vector<size_t> countOccurences() const;
};

}// namespace GRIT

#endif
