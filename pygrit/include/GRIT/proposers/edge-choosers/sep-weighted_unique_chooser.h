#ifndef GRIT_UNIQUE_SEPARATED_WEIGHTED_EDGE_CHOOSER_H
#define GRIT_UNIQUE_SEPARATED_WEIGHTED_EDGE_CHOOSER_H

#include <random>

#include "SamplableSet.hpp"
#include "hash_specialization.hpp"

#include "GRIT/utility.h"
#include "GRIT/proposers/movetypes.h"
#include "chooser_base.h"


namespace GRIT {

class SeparatedWeightedUniqueEdgeChooser: public EdgeChooserBase {
    const Observations& observations;
    const Hypergraph& hypergraph;
    double choosePairWithoutObservationsProbability;

    sset::SamplableSet<std::pair<size_t, size_t>> nonEdgesWithObservations;
    sset::SamplableSet<std::pair<size_t, size_t>> nonEdgesWithoutObservations;
    std::bernoulli_distribution choosePairWithObservationsDistribution;

    public:
        SeparatedWeightedUniqueEdgeChooser(const Observations& observations, const Hypergraph& hypergraph, double choosePairWithoutObservationsProbability=1e-5);
        Edge choose();
        double getForwardProbability(const Edge& chosenEdge, const AddRemoveMove& move) const;
        double getReverseProbability(const Edge& chosenEdge, const AddRemoveMove& move) const;
        void updateProbabilities(const Edge&, const AddRemoveMove&);
        void recomputeDistribution();
};

} //namespace GRIT

#endif
