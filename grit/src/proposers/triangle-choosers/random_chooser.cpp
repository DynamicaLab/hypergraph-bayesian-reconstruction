#include "GRIT/proposers/triangle-choosers/random_chooser.h"


namespace GRIT {

RandomTriangleChooser::RandomTriangleChooser(size_t n) : n(n){
    combinationNumber = n*(n-1)*(n-2) / 6;
}

Triplet RandomTriangleChooser::choose() {
    std::uniform_int_distribution<size_t> distribution(0, n-1);

    size_t i, j, k;

    i = distribution(generator);

    j = distribution(generator);
    while ( j == i )
        j = distribution(generator);

    k = distribution(generator);
    while ( k == i || k == j)
        k = distribution(generator);

    return Triplet {i, j, k};
}

double RandomTriangleChooser::getForwardProbability(const Triplet &, const AddRemoveMove &) const {
    return 1/(double) combinationNumber;
}

double RandomTriangleChooser::getReverseProbability(const Triplet &triplet, const AddRemoveMove &) const {
    return 1/(double) combinationNumber;
}

} //namespace GRIT
