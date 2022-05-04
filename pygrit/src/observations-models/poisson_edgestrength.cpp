#include <stdexcept>
#include "GRIT/observations-models/poisson_edgestrength.h"


namespace GRIT {

double PoissonEdgeStrengthObservationsModel::operator()(const TwoStepsEdgeProposal& proposal) const{
    size_t i=proposal.chosenEdge.first;
    size_t j=proposal.chosenEdge.second;

    const size_t& m = hypergraph.getEdgeMultiplicity(i, j);
    const double mu[3] = { parameters[mu0Index], parameters[mu0Index+1], parameters[mu0Index+2] };


    if (m == 0 && proposal.move == REMOVE)
        throw std::logic_error("Observations model: Can't remove an edge where there is none");
    if (m > 2 || m==2 && proposal.move == ADD)
        throw std::logic_error("Observations model: edge multiplicity greater than  not allowed in the observation model");
    

    double logAcceptance;
    if (proposal.move == ADD)
        logAcceptance = observations[i][j]*( log(mu[m+1]) - log(mu[m]) ) - (mu[m+1]-mu[m]);
    else
        logAcceptance = observations[i][j]*( log(mu[m-1]) - log(mu[m]) ) - (mu[m-1]-mu[m]);

    return logAcceptance;
}

double PoissonEdgeStrengthObservationsModel::getLoglikelihood() const {
    const double mu[3] = { parameters[mu0Index], parameters[mu0Index+1], parameters[mu0Index+2] };
    const size_t& n=hypergraph.getSize();

    double logLikelihood = 0;

    for (size_t i=0; i<n; i++) {
        for (size_t j=i+1; j<n; j++) {
            const double& mean = mu[hypergraph.getEdgeMultiplicity(i, j)];
            logLikelihood += observations[i][j]*log(mean) - lgamma(observations[i][j]+1) - mean;
        }
    }
    return logLikelihood;
}

} //namespace GRIT
