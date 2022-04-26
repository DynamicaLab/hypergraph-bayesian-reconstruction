#include <stdexcept>
#include "GRIT/hypergraph-models/gilbert.h"

namespace GRIT{


double GilbertGraphModel::operator()(const TwoStepsEdgeProposal &proposal) const {
    const double& q = parameters[qIndex];

    size_t i = proposal.chosenEdge.first;
    size_t j = proposal.chosenEdge.second;

    double logLikelihood = 0;

    if (proposal.move == ADD)
        logLikelihood += log(q) - log(1-q);
    else
        logLikelihood += log(1-q) - log(q);

    return logLikelihood;
}

} //namespace GRIT
