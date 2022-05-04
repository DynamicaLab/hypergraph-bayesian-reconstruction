#ifndef GRIT_WEIGHTED_TRIANGLECHOOSER_H
#define GRIT_WEIGHTED_TRIANGLECHOOSER_H

#include <random>
#include <list>
#include <vector>

#include "GRIT/utility.h"
#include "GRIT/proposers/movetypes.h"
#include "chooser_base.h"


namespace GRIT {

class WeightedTriangleChooser: public TriangleChooserBase{
    const Observations& observations;
    size_t weightSum;
    size_t n;
    
    public:
        explicit WeightedTriangleChooser(const Observations& observations);
        Triplet choose();
        double getForwardProbability(const Triplet& triplet, const AddRemoveMove&) const;
        double getReverseProbability(const Triplet& triplet, const AddRemoveMove&) const;
        void updateProbabilities(const Triplet&, const AddRemoveMove&) {};

        size_t getWeight(const Triplet& triplet) const;
        void recomputeDistribution();
    private:
        void computeWeightSum();
};

} //namespace GRIT

#endif
