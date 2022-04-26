#include <stdexcept>
#include "GRIT/hypergraph-models/independent_hyperedges.h"


namespace GRIT{

double IndependentHyperedgesModel::operator()(const FourStepsHypergraphProposal &proposal) const {
    const double& p = parameters[pIndex];
    const double& q = parameters[qIndex];

    double logLikelihood = 0;

    if (proposal.moveType == FourStepsHypergraphProposal::TRIANGLE) {
        if (proposal.k < 0)
            throw std::runtime_error("The proposed triangle is invalid. Mismatch between the proposer and the hyperedge type");

        logLikelihood += log(p) - log(1-p);
    }
    else if (proposal.moveType == FourStepsHypergraphProposal::EDGE)
        logLikelihood += log(q) - log(1-q);


    if (proposal.move == REMOVE)
        return -logLikelihood;
    return logLikelihood;
}

double IndependentHyperedgesModel::operator()(const SixStepsHypergraphProposal &proposal) const {
    const double& p = parameters[pIndex];
    const double& q = parameters[qIndex];

    double logLikelihood = 0;

    if (proposal.moveType == SixStepsHypergraphProposal::TRIANGLE)
        logLikelihood += log(p) - log(1-p);
    else if (proposal.moveType == SixStepsHypergraphProposal::EDGE)
        logLikelihood += log(q) - log(1-q);
    else if (proposal.moveType == SixStepsHypergraphProposal::HIDDEN_EDGES)
        logLikelihood += proposal.changedPairs.size() * (log(q) - log(1-q));

    if (proposal.move == REMOVE)
        return -logLikelihood;
    return logLikelihood;
}

} // namespace GRIT
