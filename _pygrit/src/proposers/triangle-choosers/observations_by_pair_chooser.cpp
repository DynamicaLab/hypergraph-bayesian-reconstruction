#include <random>

#include "GRIT/utility.h"
#include "GRIT/proposers/triangle-choosers/observations_by_pairs_chooser.h"


namespace GRIT {
using namespace std;


ObservationsPairwiseTriangleChooser::ObservationsPairwiseTriangleChooser(const Observations& observations): observations(observations), n(observations.size()){
    resizeVectors();
    recomputeDistribution();
}

void ObservationsPairwiseTriangleChooser::recomputeDistribution() {
    computeAllWeights();
    computeNormalizingConstant();
    createDiscreteDistributionObjects();
}

void ObservationsPairwiseTriangleChooser::resizeVectors(){
    vertexWeights.resize(n);
    neighbourWeights.resize(n, vector<size_t>(n));
    neighbourDistributions.resize(n);
}


void ObservationsPairwiseTriangleChooser::computeAllWeights(){
    computeNeighbourWeights();
    computeVertexWeights();
}

static inline size_t getWeight(const size_t& observations){
    return observations+1;
}

void ObservationsPairwiseTriangleChooser::computeNeighbourWeights(){
    for (size_t i=0; i<n; i++){
        for (size_t j=0; j<n; j++)
            if (i != j)
                neighbourWeights[i][j] = getWeight(observations[i][j]);
            else
                neighbourWeights[i][j] = 0;
    }
}

void ObservationsPairwiseTriangleChooser::computeVertexWeights(){
    for (size_t i=0; i<n; i++){
        vertexWeights[i] = 0;
        for (size_t j=0; j<n; j++)
            if (j != i)
                vertexWeights[i] += getWeight(observations[i][j]);
    }
}

void ObservationsPairwiseTriangleChooser::computeNormalizingConstant(){
    normalizingConstant = 0;
    for (size_t i=0; i<n; i++)
        for (size_t j=i+1; j<n; j++)
            normalizingConstant += 2*getWeight(observations[i][j]);
}

void ObservationsPairwiseTriangleChooser::createDiscreteDistributionObjects(){
    vertexDistribution = discrete_distribution<size_t>(vertexWeights.begin(), vertexWeights.end());

    for (size_t i=0; i<n; i++)
        neighbourDistributions[i] = discrete_distribution<size_t>(neighbourWeights[i].begin(), neighbourWeights[i].end());
}


Triplet ObservationsPairwiseTriangleChooser::choose(){
    size_t firstVertex = drawVertex();
    size_t secondVertex = drawAdjacentVertexTo(firstVertex);
    size_t thirdVertex = drawAdjacentVertexTo(firstVertex);

    Triplet chosenTriplet({firstVertex, secondVertex, thirdVertex});

    return chosenTriplet;
}

double ObservationsPairwiseTriangleChooser::getForwardProbability(const Triplet& triplet, const AddRemoveMove&) const{
    const size_t& i = triplet.i;
    const size_t& j = triplet.j;
    const size_t& k = triplet.k;

    double probability;

    if (i==j && i==k && j==k)
        probability = 0;
    else {
        double wi = vertexWeights[i];
        double wj = vertexWeights[j];
        double wk = vertexWeights[k];

        double wij = neighbourWeights[i][j];
        double wik = neighbourWeights[i][k];
        double wjk = neighbourWeights[j][k];

        probability = wij*(wik/wi + wjk/wj) + wjk*(wij/wj + wik/wk) + wik*(wij/wi + wjk/wk);
        probability /= (double) normalizingConstant;
    }
    return probability;
}

double ObservationsPairwiseTriangleChooser::getReverseProbability(const Triplet &triplet, const AddRemoveMove &__unusedArgument) const{
    return getForwardProbability(triplet, __unusedArgument);
}
    

size_t ObservationsPairwiseTriangleChooser::drawVertex(){
    return vertexDistribution(generator);
}

size_t ObservationsPairwiseTriangleChooser::drawAdjacentVertexTo(size_t vertex){
    return neighbourDistributions[vertex](generator);
}

} //namespace GRIT
