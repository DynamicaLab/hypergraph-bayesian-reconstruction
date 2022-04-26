#include <stdexcept>
#include <algorithm>

#include "GRIT/proposers/edge-choosers/uniform_edge_chooser.h"


namespace GRIT {


UniformNonEdgeChooser::UniformNonEdgeChooser(const Hypergraph& hypergraph): hypergraph(hypergraph), samplableSet(1, 2) {
    recomputeDistribution();
}

void UniformNonEdgeChooser::recomputeDistribution() {
    samplableSet.clear();

    for (size_t i=0; i<hypergraph.getSize(); i++)
        for (auto neighbhour: hypergraph.getEdgesFrom(i)) {
            const size_t& j = neighbhour.first;
            if (i < j)
                samplableSet.insert({i, j}, 1);
        }
}

Edge UniformNonEdgeChooser::choose() {
    return samplableSet.sample_ext_RNG<std::mt19937>(generator).first;
}

double UniformNonEdgeChooser::getForwardProbability(const Edge& edge, const AddRemoveMove&) const{
    return 1./samplableSet.size();
}

double UniformNonEdgeChooser::getReverseProbability(const Edge& edge, const AddRemoveMove& move) const{
    double probability;
    size_t currentEdgeMultiplicity = hypergraph.getEdgeMultiplicity(edge.first, edge.second);

    if (move == ADD) {
        if (currentEdgeMultiplicity == 0)
            probability = 1. / (double) (samplableSet.size() + 1);
        else
            probability = 1. / (double) samplableSet.size();
    }
    else {
        if (currentEdgeMultiplicity > 1)
            probability = 1. / (double) samplableSet.size();
        else
            probability = 0;
    }

    return probability;
}

void UniformNonEdgeChooser::updateProbabilities(const Edge& edge, const AddRemoveMove& move) {
    Edge orderedEdge(edge);
    if (edge.first > edge.second) orderedEdge = Edge {edge.second, edge.first};

    size_t currentEdgeMultiplicity = hypergraph.getEdgeMultiplicity(edge.first, edge.second);

    if (move == ADD && currentEdgeMultiplicity == 0)
        samplableSet.insert(orderedEdge, 1);

    else if (move == REMOVE && currentEdgeMultiplicity == 1)
        samplableSet.erase(orderedEdge);
}

} //namespace GRIT
