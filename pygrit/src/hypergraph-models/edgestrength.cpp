#include <stdexcept>
#include "GRIT/hypergraph-models/edgestrength.h"

namespace GRIT{


double EdgeStrengthGraphModel::operator()(const TwoStepsEdgeProposal &proposal) const {
    const double& q1 = parameters[q1Index];
    const double& q2 = parameters[q2Index];

    size_t i = proposal.chosenEdge.first;
    size_t j = proposal.chosenEdge.second;
    size_t m = hypergraph.getEdgeMultiplicity(i, j);

    double logLikelihood = 0;

    if (proposal.move == ADD) {
        if (m == 0)
            logLikelihood += log(q1) - log(1-q1);
        else if (m == 1)
            logLikelihood += log(q2) - log(1-q2) - log(q1);
        else
            throw std::logic_error("Graph model: Cannot add edge of multiplicity 2.");
    }
    else {
        if (m == 1)
            logLikelihood += log(1-q1) - log(q1);
        else if (m == 2)
            logLikelihood += log(1-q2) + log(q1) - log(q2);
        else if (m == 0)
            throw std::logic_error("Graph model: Cannot remove edge of multiplicity 0.");
    }

    return logLikelihood;
}

double EdgeStrengthGraphModel::getLoglikelihood() const {
    const double& q1 = parameters[q1Index];
    const double& q2 = parameters[q2Index];

    const size_t& n = hypergraph.getSize();
    size_t weakEdges = 0;
    size_t strongEdges = 0;

    for (size_t i=0; i<n-1; i++) {
        for (auto& edge_multiplicity: hypergraph.getEdgesFrom(i)) {
            if (edge_multiplicity.first <= i) continue;

            if (edge_multiplicity.second == 1)
                weakEdges++;
            else if (edge_multiplicity.second == 2)
                strongEdges++;
        }
    }
    double logLikelihood = 0;

    logLikelihood += ( nchoose2(n)-weakEdges-strongEdges ) * ( log(1-q1) + log(1-q2) );
    logLikelihood += weakEdges*( log(q1) + log(1-q2) );
    logLikelihood += strongEdges*( log(q2) );
    return logLikelihood;
}

} //namespace GRIT
