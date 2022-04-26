#ifndef GRIT_TRIANGLE_CHOOSER_BASE_H
#define GRIT_TRIANGLE_CHOOSER_BASE_H


#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"
#include "GRIT/proposers/movetypes.h"


namespace GRIT{

class TriangleChooserBase {
    public:
        virtual Triplet choose() = 0;
        virtual ~TriangleChooserBase() {};

        virtual double getForwardProbability(const Triplet& triplet, const AddRemoveMove&) const = 0;
        virtual double getReverseProbability(const Triplet& triplet, const AddRemoveMove&) const = 0;
        virtual void updateProbabilities(const Triplet&, const AddRemoveMove&) = 0;
        virtual void recomputeDistribution() = 0;

};

} // namespace GRIT

#endif
