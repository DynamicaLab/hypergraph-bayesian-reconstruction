#include <stdexcept>
#include "GRIT/hypergraph-models/gilbert.h"

namespace GRIT{


double GilbertGraphModel::operator()(const TwoStepsEdgeProposal &proposal) const {
    const double& q = parameters[qIndex];

    if (proposal.move == ADD)
        return log(q) - log(1-q);
    else
        return log(1-q) - log(q);
}

} //namespace GRIT
