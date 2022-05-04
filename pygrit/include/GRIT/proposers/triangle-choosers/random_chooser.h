#ifndef GRIT_RANDOM_TRIANGLECHOOSER_H
#define GRIT_RANDOM_TRIANGLECHOOSER_H

#include <random>
#include <list>
#include <vector>

#include "GRIT/utility.h"
#include "GRIT/proposers/movetypes.h"
#include "chooser_base.h"


namespace GRIT {

class RandomTriangleChooser: public TriangleChooserBase{
    size_t combinationNumber;
    size_t n;
    
    public:
        explicit RandomTriangleChooser(size_t n);
        Triplet choose();
        double getForwardProbability(const Triplet& triplet, const AddRemoveMove&) const;
        double getReverseProbability(const Triplet& triplet, const AddRemoveMove&) const;
        void updateProbabilities(const Triplet&, const AddRemoveMove&) {};
        void recomputeDistribution() {};
};

} //namespace GRIT

#endif
