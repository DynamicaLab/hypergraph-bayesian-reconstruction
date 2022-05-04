#ifndef GRIT_EDGE_CHOOSER_BASE_H
#define GRIT_EDGE_CHOOSER_BASE_H


#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"
#include "GRIT/proposers/movetypes.h"


namespace GRIT{

class EdgeChooserBase {
    public:
        virtual Edge choose() = 0;
        virtual ~EdgeChooserBase() {};

        virtual double getForwardProbability(const Edge& triplet, const AddRemoveMove&) const = 0;
        virtual double getReverseProbability(const Edge& triplet, const AddRemoveMove&) const = 0;
        virtual void updateProbabilities(const Edge&, const AddRemoveMove&) = 0;
        virtual void recomputeDistribution() = 0;

};

} // namespace GRIT

#endif
