#ifndef GRIT_INDEPENDENT_EDGETYPES_MODEL_H
#define GRIT_INDEPENDENT_EDGETYPES_MODEL_H

#include <math.h>

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"
#include "GRIT/proposers/movetypes.h"


namespace GRIT {

class EdgeStrengthGraphModel {
    const Hypergraph& hypergraph;
    const Parameters& parameters;
    const size_t q1Index = 0;
    const size_t q2Index = 1;

    public:
        EdgeStrengthGraphModel(const Hypergraph& hypergraph, const Parameters& parameters, const Observations& observations):
            hypergraph(hypergraph), parameters(parameters) {}

        double operator()(const TwoStepsEdgeProposal& proposal) const;
        double getLoglikelihood() const;

};

} //namespace GRIT

#endif
