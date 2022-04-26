#include <stdexcept>

#include "GRIT/proposers/edge-choosers/weighted_unique_chooser.h"


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

ObservationsWeightedUniqueEdgeChooser::ObservationsWeightedUniqueEdgeChooser(const Observations& observations, const Hypergraph& hypergraph): observations(observations), samplableSet(1, findDataMaximum(observations)+1), hypergraph(hypergraph){
    for (size_t i=0; i<observations.size(); i++)
        for (size_t j=i+1; j<observations.size(); j++)
            if (!hypergraph.isEdge(i, j))
                samplableSet.insert({i, j}, observations[i][j]+1);
}

void ObservationsWeightedUniqueEdgeChooser::recomputeDistribution() {
    samplableSet = sset::SamplableSet<std::pair<size_t, size_t>>(1, findDataMaximum(observations)+1);

    for (size_t i=0; i<observations.size(); i++)
        for (size_t j=i+1; j<observations.size(); j++)
            if (!hypergraph.isEdge(i, j))
                samplableSet.insert({i, j}, observations[i][j]+1);
}

Edge ObservationsWeightedUniqueEdgeChooser::choose() {
    return samplableSet.sample_ext_RNG<std::mt19937>(generator).first;
}

double ObservationsWeightedUniqueEdgeChooser::getForwardProbability(const Edge& edge, const AddRemoveMove&) const{
    size_t weight = observations[edge.first][edge.second]+1;

    return weight/ (double) samplableSet.total_weight();
}

double ObservationsWeightedUniqueEdgeChooser::getReverseProbability(const Edge& edge, const AddRemoveMove& move) const{
    size_t weight = observations[edge.first][edge.second]+1;
    size_t currentEdgeMultiplicity = hypergraph.getEdgeMultiplicity(edge.first, edge.second);

    if (move == REMOVE && currentEdgeMultiplicity == 1)
        return weight/ ((double) samplableSet.total_weight() + (double) weight);
    return 0;
}

void ObservationsWeightedUniqueEdgeChooser::updateProbabilities(const Edge& edge, const AddRemoveMove& move) {
    Edge orderedEdge(edge);
    if (edge.first > edge.second) orderedEdge = Edge {edge.second, edge.first};

    size_t currentEdgeMultiplicity = hypergraph.getEdgeMultiplicity(edge.first, edge.second);

    if (move == ADD && currentEdgeMultiplicity == 0)
        samplableSet.erase(orderedEdge);

    else if (move == REMOVE && currentEdgeMultiplicity == 1)
        samplableSet.insert(orderedEdge, observations[orderedEdge.first][orderedEdge.second]+1);
}

} //namespace GRIT
