#ifndef GRIT_GILBERT_GRAPH_MODEL_H
#define GRIT_GILBERT_GRAPH_MODEL_H

#include <math.h>

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"
#include "GRIT/proposers/movetypes.h"


namespace GRIT {

class GilbertGraphModel {
    const Hypergraph& hypergraph;
    const Parameters& parameters;
    const size_t qIndex = 0;

    public:
        GilbertGraphModel(const Hypergraph& hypergraph, const Parameters& parameters, const Observations& observations):
            hypergraph(hypergraph), parameters(parameters) {}

        double operator()(const TwoStepsEdgeProposal& proposal) const;
        double getLoglikelihood() const {
            const double& q = parameters[qIndex];
            const size_t& m = hypergraph.getEdgeNumber();
            const size_t& n = hypergraph.getSize();
            return m*log(q) + (nchoose2(n)-m)*log(1-q);
        }

};

} //namespace GRIT

#endif
