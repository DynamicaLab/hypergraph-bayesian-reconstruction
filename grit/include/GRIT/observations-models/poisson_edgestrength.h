#ifndef GRIT_EDGETYPES_DATAMODEL_H
#define GRIT_EDGETYPES_DATAMODEL_H


#include <stdexcept>

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"
#include "GRIT/proposers/movetypes.h"


namespace GRIT {

class PoissonEdgeStrengthObservationsModel {
    const Observations& observations;
    const Hypergraph& hypergraph;
    const Parameters& parameters;
    const size_t mu0Index = 2;

    public:
        PoissonEdgeStrengthObservationsModel(const Hypergraph& hypergraph, const Parameters& parameters, const Observations& observations):
            observations(observations), hypergraph(hypergraph), parameters(parameters) {}

        double operator()(const TwoStepsEdgeProposal& proposal) const;

        double getLoglikelihood() const;
};

} //namespace GRIT

#endif
