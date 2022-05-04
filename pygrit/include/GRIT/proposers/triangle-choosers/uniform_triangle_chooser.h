#ifndef GRIT_UNIFORM_TRIANGLE_REMOVAL_H
#define GRIT_UNIFORM_TRIANGLE_REMOVAL_H


#include "SamplableSet.hpp"

#include "GRIT/utility.h"
#include "GRIT/proposers/movetypes.h"
#include "chooser_base.h"


namespace GRIT {

class UniformTriangleChooser: public TriangleChooserBase{
    const Hypergraph& hypergraph;
    sset::SamplableSet<size_t> samplableSet;

    public:
        explicit UniformTriangleChooser(const Hypergraph& hypergraph);
        Triplet choose();
        double getForwardProbability(const Triplet&, const AddRemoveMove&) const;
        double getReverseProbability(const Triplet&, const AddRemoveMove&) const;
        void updateProbabilities(const Triplet& triplet, const AddRemoveMove& move);
        void recomputeDistribution();
    private:
        void buildSamplableSetFromGraph();
        void updateWeightOfIndex(const size_t& index, const AddRemoveMove& move);

        std::pair<size_t, size_t> drawTriangleUniformelyFromVertex(size_t vertex) const;
};

} //namespace GRIT

#endif
