#ifndef GRIT_UNIFORM_NONZERO_EDGE_CHOOSER_H
#define GRIT_UNIFORM_NONZERO_EDGE_CHOOSER_H

#include "SamplableSet.hpp"
#include "hash_specialization.hpp"

#include "GRIT/utility.h"
#include "GRIT/proposers/movetypes.h"
#include "chooser_base.h"


namespace GRIT {

class UniformNonEdgeChooser: public EdgeChooserBase {
    const Hypergraph& hypergraph;
    sset::SamplableSet<std::pair<size_t, size_t>> samplableSet;

    public:
        UniformNonEdgeChooser(const Hypergraph& hypergraph);
        Edge choose();
        double getForwardProbability(const Edge& chosenEdge, const AddRemoveMove& move) const;
        double getReverseProbability(const Edge& chosenEdge, const AddRemoveMove& move) const;
        void updateProbabilities(const Edge&, const AddRemoveMove&);
        void recomputeDistribution();
};

} //namespace GRIT

#endif
