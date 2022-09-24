#ifndef GRIT_HYPERGRAPH_POISSON_DATAMODEL_H
#define GRIT_HYPERGRAPH_POISSON_DATAMODEL_H


#include <stdexcept>

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"
#include "GRIT/proposers/movetypes.h"


namespace GRIT {

class PoissonHypergraphObservationsModel {
    const Observations& observations;
    const Hypergraph& hypergraph;
    const Parameters& parameters;
    const size_t mu0Index = 2;

    public:
        PoissonHypergraphObservationsModel(const Hypergraph& hypergraph, const Parameters& parameters, const Observations& observations):
            observations(observations), hypergraph(hypergraph), parameters(parameters) {}

        double operator()(const FourStepsHypergraphProposal& proposal) const;
        double operator()(const SixStepsHypergraphProposal& proposal) const;
        double getLoglikelihood() const;

    private:
        double getTriangleContribution(const Triplet& triplet, const AddRemoveMove& move) const;
        double getEdgeContribution(c_Index& i, c_Index& j, const AddRemoveMove& move) const;
};

} //namespace GRIT

#endif
