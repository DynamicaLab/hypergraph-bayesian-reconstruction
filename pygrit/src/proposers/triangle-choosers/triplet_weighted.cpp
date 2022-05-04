#include "GRIT/proposers/triangle-choosers/triplet_weighted.h"


namespace GRIT {

WeightedTriangleChooser::WeightedTriangleChooser(const Observations& observations): observations(observations), n(observations.size()) {
    computeWeightSum();
}

void WeightedTriangleChooser::recomputeDistribution() {
    computeWeightSum();
}

void WeightedTriangleChooser::computeWeightSum() {
    weightSum = 0;
    for (size_t i=0; i<n-2; i++)
        for (size_t j=i+1; j<n-1; j++)
            for (size_t k=j+1; k<n; k++)
                weightSum += getWeight({i, j, k});
}

size_t WeightedTriangleChooser::getWeight(const Triplet &triplet) const {
    return observations[triplet.i][triplet.j] + observations[triplet.i][triplet.k] + observations[triplet.j][triplet.k] + 1;
}

Triplet WeightedTriangleChooser::choose() {
    size_t chosenPosition = std::uniform_int_distribution<>(0, weightSum-1)(generator);
    size_t currentPosition = 0;
    
    Triplet chosenTriplet;
    bool found = false;
    for (size_t i=0; i<n-2 && !found; i++) {
        for (size_t j=i+1; j<n-1 && !found; j++) {
            for (size_t k=j+1; k<n && !found; k++) {
                currentPosition += getWeight({i, j, k});
                if (currentPosition >= chosenPosition) {
                    found = true;
                    chosenTriplet = Triplet {i, j, k};
                }
            }
        }
    }
    return chosenTriplet;
}

double WeightedTriangleChooser::getForwardProbability(const Triplet& triplet, const AddRemoveMove&) const {
    return getWeight(triplet) / (double) weightSum;
}

double WeightedTriangleChooser::getReverseProbability(const Triplet& triplet, const AddRemoveMove& __unusedArgument) const {
    return getForwardProbability(triplet, __unusedArgument);
}

} //namespace GRIT
