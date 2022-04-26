#include <stdexcept>

#include "GRIT/proposers/edge-choosers/weighted_chooser.h"


namespace GRIT {

static size_t findDataMaximum(const Observations& observations){
    size_t maximum = 0;
    for (size_t i=0; i<observations.size(); i++) {
        for (size_t j=i+1; j<observations.size(); j++) {
            if (observations[i][j] > maximum)
                maximum = observations[i][j] + 1;
        }
    }
    return maximum;
}

ObservationsWeightedEdgeChooser::ObservationsWeightedEdgeChooser(const Observations& observations): observations(observations), samplableSet(1, 2){
    recomputeDistribution();
}

void ObservationsWeightedEdgeChooser::recomputeDistribution() {
    samplableSet = sset::SamplableSet<std::pair<size_t, size_t>>(1, findDataMaximum(observations)+1);

    Edge edge {0, 0};
    for (size_t i=0; i<observations.size(); i++) {
        for (size_t j=i+1; j<observations.size(); j++) {
            edge = Edge {i, j};
            samplableSet.insert(edge, observations[i][j]+1);
        }
    }
}

Edge ObservationsWeightedEdgeChooser::choose() {
    return samplableSet.sample_ext_RNG<std::mt19937>(generator).first;
}

double ObservationsWeightedEdgeChooser::getForwardProbability(const Edge& edge, const AddRemoveMove&) const{
    size_t weight = observations[edge.first][edge.second]+1;
    return weight/ (double) samplableSet.total_weight();
}

double ObservationsWeightedEdgeChooser::getReverseProbability(const Edge& edge, const AddRemoveMove& __unusedArg2) const{
    return getForwardProbability(edge, __unusedArg2);
}

} //namespace GRIT
