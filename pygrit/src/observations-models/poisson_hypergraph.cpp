#include <cmath>
#include <stdexcept>
#include "GRIT/observations-models/poisson_hypergraph.h"


using namespace std;

namespace GRIT {

double PoissonHypergraphObservationsModel::operator()(const FourStepsHypergraphProposal& proposal) const {
    double logAcceptance = 0;

    if (proposal.moveType == FourStepsHypergraphProposal::TRIANGLE) {
        if (proposal.k < 0) throw runtime_error("The proposed triangle is invalid. Mismatch between the proposer and the hyperedge type");
       logAcceptance += getTriangleContribution( {proposal.i, proposal.j, (size_t) proposal.k}, proposal.move );
    }
    else
       logAcceptance += getEdgeContribution( proposal.i, proposal.j, proposal.move );

    return logAcceptance;
}

double PoissonHypergraphObservationsModel::operator()(const SixStepsHypergraphProposal& proposal) const {
    const size_t& i = proposal.chosenTriplet.i;
    const size_t& j = proposal.chosenTriplet.j;
    const size_t& k = proposal.chosenTriplet.k;

    double logAcceptance = 0;

    if (proposal.moveType == SixStepsHypergraphProposal::TRIANGLE) {
        logAcceptance += getTriangleContribution( {i, j, k}, proposal.move );
    }
    else if (proposal.moveType == SixStepsHypergraphProposal::EDGE)
       logAcceptance += getEdgeContribution(i, j, proposal.move );
    // HIDDEN_EDGES move has not impact on the observations probability

    return logAcceptance;
}

double PoissonHypergraphObservationsModel::getTriangleContribution(const Triplet& triplet, const AddRemoveMove& move) const {
    const double mu[3] = { parameters[mu0Index], parameters[mu0Index+1], parameters[mu0Index+2] };

    size_t proposalEdgeType;
    list<pair<size_t, size_t>> pairsInTriangle ({ {triplet.i, triplet.j}, {triplet.i, triplet.k}, {triplet.j, triplet.k} });

    double logLikelihood = 0;


    for (auto it=pairsInTriangle.begin(); it!=pairsInTriangle.end(); it++) {
        if (move == REMOVE)  // skips the removed triplet
            proposalEdgeType = hypergraph.getHighestOrderHyperedgeExcluding(it->first, it->second, triplet);
        else
            proposalEdgeType = hypergraph.getHighestOrderHyperedgeWith(it->first, it->second);


        if (proposalEdgeType != 2)
            logLikelihood += observations[it->first][it->second]*( log(mu[proposalEdgeType]) - log(mu[2]) ) - (mu[proposalEdgeType]-mu[2]);
    }


    if (move == ADD)
        logLikelihood = -logLikelihood;

    if (std::isnan(logLikelihood))
        throw runtime_error("ObservationModel: logLikelihood is NaN for triangle proposition. Means are "
                +std::to_string(mu[0]) + ", " + std::to_string(mu[1]) + ", " + std::to_string(mu[2]) + ".");

    return logLikelihood;
}

double PoissonHypergraphObservationsModel::getEdgeContribution(c_Index& i, c_Index &j, const AddRemoveMove& move) const {
    const double mu[3] = { parameters[mu0Index], parameters[mu0Index+1], parameters[mu0Index+2] };


    size_t proposalEdgeType;
    if (move == REMOVE)
        proposalEdgeType = hypergraph.getHighestOrderHyperedgeExcluding(i, j, Edge {(size_t) i, (size_t) j});
    else
        proposalEdgeType = hypergraph.getHighestOrderHyperedgeWith(i, j);

    double logLikelihood = 0;

    if (proposalEdgeType == 0)
        logLikelihood += observations[i][j]*( log(mu[0]) - log(mu[1]) ) - (mu[0] - mu[1]);

    if (move == ADD)
        logLikelihood = -logLikelihood;

    if (std::isnan(logLikelihood))
        throw runtime_error("ObservationModel: logLikelihood is NaN for edge proposition. Means are "
                +std::to_string(mu[0]) + ", " + std::to_string(mu[1]) + ", " + std::to_string(mu[2]) + ".");

    return logLikelihood;
}


double PoissonHypergraphObservationsModel::getLoglikelihood() const{
    const size_t& n = hypergraph.getSize();
    const double mu[3] = { parameters[mu0Index], parameters[mu0Index+1], parameters[mu0Index+2] };

    double logLikelihood = 0;

    for (size_t i=0; i<n; i++) {
        for (size_t j=i+1; j<n; j++) {
            const double& mean = mu[hypergraph.getHighestOrderHyperedgeWith(i, j)];
            logLikelihood += observations[i][j]*log(mean) - lgamma(observations[i][j]+1) - mean;
        }
    }
    return logLikelihood;
}

} //namespace GRIT
