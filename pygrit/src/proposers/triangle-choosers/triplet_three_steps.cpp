#include <random>

#include "GRIT/utility.h"
#include "GRIT/proposers/triangle-choosers/triplet_three_steps.h"


namespace GRIT {
using namespace std;


TriangleThreeStepsChooser::TriangleThreeStepsChooser(const Observations& observations): observations(observations), n(observations.size()){
    resizeVectors();
    computeAllWeights();
    computeNormalizingConstant();
    createDiscreteDistributionObjects();
}

void TriangleThreeStepsChooser::recomputeDistribution() {
    computeAllWeights();
    computeNormalizingConstant();
    createDiscreteDistributionObjects();
}

void TriangleThreeStepsChooser::resizeVectors(){
    vertexWeights.resize(n);
    neighbourWeights.resize(n, vector<size_t>(n));
    neighbourDistributions.resize(n);
}


void TriangleThreeStepsChooser::computeAllWeights(){
    computeNeighbourWeights();
    computeVertexWeights();
}

static size_t getWeight(size_t observations){
    return pow(observations + 1, 2);
}

void TriangleThreeStepsChooser::computeNeighbourWeights(){
    for (size_t i=0; i<n; i++){
        for (size_t j=0; j<n; j++){
            if (i != j)
                neighbourWeights[i][j] = getWeight(observations[i][j]);
            else
                neighbourWeights[i][j] = 0;
        }
    }
}

void TriangleThreeStepsChooser::computeVertexWeights(){
    for (size_t i=0; i<n; i++){
        vertexWeights[i] = 0;
        for (size_t j=0; j<n; j++){
            if (j != i)
                vertexWeights[i] += getWeight(observations[i][j]);
        }
    }
}

void TriangleThreeStepsChooser::computeNormalizingConstant(){
    normalizingConstant = 0;
    for (size_t i=0; i<n; i++)
        for (size_t j=i+1; j<n; j++)
            normalizingConstant += 2*getWeight(observations[i][j]);
}

void TriangleThreeStepsChooser::createDiscreteDistributionObjects(){
    vertexDistribution = discrete_distribution<size_t>(vertexWeights.begin(), vertexWeights.end());

    for (size_t i=0; i<n; i++)
        neighbourDistributions[i] = discrete_distribution<size_t>(neighbourWeights[i].begin(), neighbourWeights[i].end());
}

Triplet TriangleThreeStepsChooser::choose(){
    size_t firstVertex = drawVertex();
    size_t secondVertex = drawSecondVertex(firstVertex);
    size_t thirdVertex = drawThirdVertex(firstVertex, secondVertex);


    Triplet chosenTriplet({firstVertex, secondVertex, thirdVertex});

    return chosenTriplet;
}

double TriangleThreeStepsChooser::getForwardProbability(const Triplet& triplet, const AddRemoveMove&) const{
    size_t tripletArray[3] {triplet.i, triplet.j, triplet.k};

    double probability = 0;

    if (triplet.i==triplet.j || triplet.i==triplet.k || triplet.j==triplet.k) {}
    else {
        double thirdVertexProbability;

        for (size_t k=0; k<3; k++) {
            thirdVertexProbability = 0;
            for (size_t i=0; i<3; i++) {
                for (size_t j=0; j<3; j++) {
                    if (i!=j && j!=k && i!=k) {
                        const double& wij = neighbourWeights[tripletArray[i]][tripletArray[j]];
                        if (thirdVertexProbability == 0)
                            thirdVertexProbability = getThirdVertexProbability(tripletArray[i], tripletArray[j], tripletArray[k]);

                        probability += wij*thirdVertexProbability;
                    }
                }
            }
        }
        probability /= (double) normalizingConstant;
    }
    return probability;
}

double TriangleThreeStepsChooser::getReverseProbability(const Triplet &triplet, const AddRemoveMove &__unusedArgument) const{
    return getForwardProbability(triplet, __unusedArgument);
}
    

size_t TriangleThreeStepsChooser::drawVertex(){
    return vertexDistribution(generator);
}

size_t TriangleThreeStepsChooser::drawSecondVertex(size_t vertex){
    return neighbourDistributions[vertex](generator);
}

size_t TriangleThreeStepsChooser::drawThirdVertex(size_t i, size_t j) {
    vector<size_t> weights(n, 0);

    for (size_t k=0; k<n; k++) {
        if (k != i && k != j)
            weights[k] = observations[i][k] + observations[j][k] + 1;
    }

    discrete_distribution<size_t> distribution(weights.begin(), weights.end());
    return distribution(generator);
}

double TriangleThreeStepsChooser::getThirdVertexProbability(size_t i, size_t j, size_t k) const {
    double probability = 0;

    if (i != j && i!=k && j!=k) {
        size_t weightSum = 0;

        for (size_t ell=0; ell<n; ell++) {
            if (ell != i && ell != j)
                weightSum += observations[i][ell] + observations[j][ell] + 1;
        }
        probability = (observations[i][k] + observations[j][k] + 1)/ (double) weightSum;
    }
    return probability;
}

} //namespace GRIT
