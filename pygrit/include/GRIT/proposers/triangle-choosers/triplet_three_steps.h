#ifndef GRIT_THREESTEPS_TRIPLET_TRIANGLECHOOSER_H
#define GRIT_THREESTEPS_TRIPLET_TRIANGLECHOOSER_H

#include <random>
#include <list>
#include <vector>

#include "GRIT/utility.h"
#include "GRIT/proposers/movetypes.h"
#include "chooser_base.h"


namespace GRIT {

class TriangleThreeStepsChooser: public TriangleChooserBase{
    const Observations& observations;
    size_t n;
    size_t normalizingConstant;
    std::vector<std::vector<size_t>> neighbourWeights;
    std::vector<size_t> vertexWeights;

    std::vector<std::discrete_distribution<size_t>> neighbourDistributions;
    std::discrete_distribution<size_t> vertexDistribution;

    public:
        explicit TriangleThreeStepsChooser(const Observations& observations);
        Triplet choose();
        double getForwardProbability(const Triplet& triplet, const AddRemoveMove&) const;
        double getReverseProbability(const Triplet& triplet, const AddRemoveMove&) const;
        void updateProbabilities(const Triplet&, const AddRemoveMove&) {};
        void recomputeDistribution();
    private:
        void resizeVectors();

        void computeAllWeights();
        void computeVertexWeights();
        void computeNeighbourWeights();
        void computeNormalizingConstant();

        void createDiscreteDistributionObjects();

        size_t drawVertex();
        size_t drawSecondVertex(size_t vertex);
        size_t drawThirdVertex(size_t vertex1, size_t vertex2);
        double getThirdVertexProbability(size_t i, size_t j, size_t k) const;
};

} //namespace GRIT

#endif
