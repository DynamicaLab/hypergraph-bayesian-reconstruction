#include <stdexcept>

#include "GRIT/proposers/edge-choosers/weighted_two-layers_chooser.h"


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

TwoLayersObservationsWeightedEdgeChooser::TwoLayersObservationsWeightedEdgeChooser(const Observations& observations, const Hypergraph& hypergraph): observations(observations), hypergraph(hypergraph), samplableSet(1, 2) {
    recomputeDistribution();
}

void TwoLayersObservationsWeightedEdgeChooser::recomputeDistribution() {
    samplableSet = sset::SamplableSet<std::pair<size_t, size_t>>(1, findDataMaximum(observations)+1);
    size_t edgeMultiplicity = 0;

    for (size_t i=0; i<hypergraph.getSize(); i++)
        for (size_t j=i+1; j<hypergraph.getSize(); j++) {
            edgeMultiplicity = hypergraph.getEdgeMultiplicity(i, j);

            if (edgeMultiplicity < 2)
                samplableSet.insert({i, j}, observations[i][j]+1);
            else if (edgeMultiplicity > 2)
                throw std::logic_error("An edge multiplicity in the initial hypergraph is greater than 2, it is not compatible with this chooser");
        }
}

Edge TwoLayersObservationsWeightedEdgeChooser::choose() {
    return samplableSet.sample_ext_RNG<std::mt19937>(generator).first;
}

double TwoLayersObservationsWeightedEdgeChooser::getForwardProbability(const Edge& edge, const AddRemoveMove&) const{
    size_t weight = observations[edge.first][edge.second]+1;
    return weight/ (double) samplableSet.total_weight();
}

double TwoLayersObservationsWeightedEdgeChooser::getReverseProbability(const Edge& edge, const AddRemoveMove& move) const{
    double probability;

    size_t edgeMultiplicity = hypergraph.getEdgeMultiplicity(edge.first, edge.second);
    size_t weight = observations[edge.first][edge.second]+1;

    if (move == REMOVE) {
        if (edgeMultiplicity == 2)
            probability = weight/ ( (double) samplableSet.total_weight() + weight);
        else if(edgeMultiplicity < 2)
            probability = getForwardProbability(edge, move);
        else
            probability = 0;
    }
    else {
        if (edgeMultiplicity >= 1)
            probability = 0;
        else
            probability = getForwardProbability(edge, move);
    }

    return probability;
}

void TwoLayersObservationsWeightedEdgeChooser::updateProbabilities(const Edge& edge, const AddRemoveMove& move) {
    Edge orderedEdge(edge);
    if (edge.first > edge.second) orderedEdge = Edge {edge.second, edge.first};

    size_t edgeMultiplicity = hypergraph.getEdgeMultiplicity(edge.first, edge.second);


    if (edgeMultiplicity == 2 && move == REMOVE)
        samplableSet.insert(orderedEdge, observations[edge.first][edge.second]+1);

    else if (edgeMultiplicity == 1 && move == ADD)
        samplableSet.erase(orderedEdge);
}

} //namespace GRIT
