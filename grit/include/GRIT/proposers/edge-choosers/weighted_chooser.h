#ifndef GRIT_EDGE_DATAWEIGHTED_CHOOSER_H
#define GRIT_EDGE_DATAWEIGHTED_CHOOSER_H

#include <random>

#include "SamplableSet.hpp"
#include "hash_specialization.hpp"

#include "GRIT/utility.h"
#include "GRIT/proposers/movetypes.h"
#include "chooser_base.h"


namespace GRIT {

class ObservationsWeightedEdgeChooser: public EdgeChooserBase {
    const Observations& observations;
    sset::SamplableSet<std::pair<size_t, size_t>> samplableSet;

    public:
        ObservationsWeightedEdgeChooser(const Observations& data);
        Edge choose();
        double getForwardProbability(const Edge& chosenEdge, const AddRemoveMove& move) const;
        double getReverseProbability(const Edge& chosenEdge, const AddRemoveMove& move) const;
        void updateProbabilities(const Edge&, const AddRemoveMove&) {};
        void recomputeDistribution();
};

} //namespace GRIT

#endif
