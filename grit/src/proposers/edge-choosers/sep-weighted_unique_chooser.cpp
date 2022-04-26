#include <stdexcept>

#include "GRIT/proposers/edge-choosers/sep-weighted_unique_chooser.h"


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

SeparatedWeightedUniqueEdgeChooser::SeparatedWeightedUniqueEdgeChooser(const Observations& observations, const Hypergraph& hypergraph, double choosePairWithoutObservationsProbability):
        observations(observations), hypergraph(hypergraph), choosePairWithoutObservationsProbability(choosePairWithoutObservationsProbability),
        choosePairWithObservationsDistribution(1-choosePairWithoutObservationsProbability),
        nonEdgesWithObservations(1, findDataMaximum(observations)+1), nonEdgesWithoutObservations(1, 1) { recomputeDistribution(); }

void SeparatedWeightedUniqueEdgeChooser::recomputeDistribution() {
    nonEdgesWithObservations = sset::SamplableSet<std::pair<size_t, size_t>>(1, findDataMaximum(observations)+1);
    nonEdgesWithoutObservations = sset::SamplableSet<std::pair<size_t, size_t>>(1, findDataMaximum(observations)+1);

    for (size_t i=0; i<observations.size(); i++)
        for (size_t j=i+1; j<observations.size(); j++)
            if (!hypergraph.isEdge(i, j)) {
                if (observations[i][j] == 0)
                    nonEdgesWithoutObservations.insert({i, j}, 1);
                else
                    nonEdgesWithObservations.insert({i, j}, observations[i][j]+1);
            }
}

Edge SeparatedWeightedUniqueEdgeChooser::choose() {
    if (nonEdgesWithoutObservations.size() == 0 || choosePairWithObservationsDistribution(generator) && nonEdgesWithObservations.size() > 0)
        return nonEdgesWithObservations.sample_ext_RNG<std::mt19937>(generator).first;
    else
        return nonEdgesWithoutObservations.sample_ext_RNG<std::mt19937>(generator).first;
}

double SeparatedWeightedUniqueEdgeChooser::getForwardProbability(const Edge& edge, const AddRemoveMove&) const{
    size_t weight = observations[edge.first][edge.second]+1;

    if (weight > 1) {
        double prob = weight/ (double) nonEdgesWithObservations.total_weight();

        if (nonEdgesWithoutObservations.size() > 0)
            prob *= (1-choosePairWithoutObservationsProbability);
        return prob;
    }
    else {
        double prob = weight/ (double) nonEdgesWithoutObservations.total_weight();

        if (nonEdgesWithObservations.size() > 0)
            prob *= choosePairWithoutObservationsProbability;
        return prob;
    }
}

double SeparatedWeightedUniqueEdgeChooser::getReverseProbability(const Edge& edge, const AddRemoveMove& move) const{
    size_t weight = observations[edge.first][edge.second]+1;
    size_t currentEdgeMultiplicity = hypergraph.getEdgeMultiplicity(edge.first, edge.second);

    if (move == REMOVE && currentEdgeMultiplicity == 1) {
        if (weight > 1) {
            double prob = weight/ ( (double) nonEdgesWithObservations.total_weight() + weight );

            if (nonEdgesWithoutObservations.size() > 0)
                prob *= (1-choosePairWithoutObservationsProbability);
            return prob;
        }
        else {
            double prob = weight/ ( (double) nonEdgesWithoutObservations.total_weight() + weight );

            if (nonEdgesWithObservations.size() > 0)
                prob *= choosePairWithoutObservationsProbability;
            return prob;
        }
    }
    return 0;
}

void SeparatedWeightedUniqueEdgeChooser::updateProbabilities(const Edge& edge, const AddRemoveMove& move) {
    Edge orderedEdge(edge);
    size_t weight = observations[edge.first][edge.second]+1;
    if (edge.first > edge.second) orderedEdge = Edge {edge.second, edge.first};

    size_t currentEdgeMultiplicity = hypergraph.getEdgeMultiplicity(edge.first, edge.second);

    if (move == ADD && currentEdgeMultiplicity == 0) {
        if (weight == 1)
            nonEdgesWithoutObservations.erase(orderedEdge);
        else
            nonEdgesWithObservations.erase(orderedEdge);
    }

    else if (move == REMOVE && currentEdgeMultiplicity == 1) {
        if (weight == 1)
            nonEdgesWithoutObservations.insert(orderedEdge, weight);
        else
            nonEdgesWithObservations.insert(orderedEdge, weight);
    }
}

} //namespace GRIT
