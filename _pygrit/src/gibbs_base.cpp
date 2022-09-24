#include <cstddef>
#include <fstream>
#include <random>
#include <sstream>

#include "GRIT/utility.h"
#include "GRIT/gibbs_base.h"


namespace GRIT {


void GibbsBase::writeStateToFile(size_t iteration) const {
    writeGraphStateToBinary(iteration);
    writeParametersStateToBinary(iteration);
}

void GibbsBase::writeGraphStateToBinary(size_t iteration) const {
    std::stringstream hypergraphFileName;

    hypergraphFileName << hypergraphSampleDirectory << hypergraphSamplePrefix << chainID << '_' << iteration << ".bin";
    hypergraph.writeToBinary(hypergraphFileName.str());
}

void GibbsBase::writeParametersStateToBinary(size_t iteration) const {
    std::stringstream parametersFileName;
    parametersFileName << parameterSampleDirectory << parametersSamplePrefix << chainID << '_' << iteration << ".bin";

    writeParametersToBinary(parameters, parametersFileName.str());
}

void GibbsBase::sample(size_t sampleSize, size_t burnin) {

    resetValues();
    outputProgressToConsole(0, sampleSize, burnin);
    for (size_t i=0; i<sampleSize+burnin; i++) {

        sampleFromPosterior();
        if (i >= burnin) {
            writeStateToFile(i-burnin);
        }
        outputProgressToConsole(i+1, sampleSize, burnin);
    }
}

static void updateParametersAverage(const GRIT::Parameters& sampleParameters, GRIT::Parameters& averageParameters, size_t chainSize) {
    if (chainSize == 1)
        averageParameters = sampleParameters;
    else
        for (size_t i=0; i<averageParameters.size(); i++)
            averageParameters[i] += (sampleParameters[i] - averageParameters[i])/chainSize;
}

RandomVariables GibbsBase::sampleAndGetAverage(size_t sampleSize, size_t burnin, bool correlation, bool writeSamplesToFile) {
    resetValues();
    outputProgressToConsole(0, sampleSize, burnin);

    // No edge (type 0) is deduced from the others.
    EdgeTypeFrequencies edgetype1(hypergraph.getSize());
    EdgeTypeFrequencies edgetype2(hypergraph.getSize());
    Parameters averageParameters;

    for (size_t i=0; i<sampleSize+burnin; i++) {
        sampleFromPosterior();

        if (i >= burnin) {
            updateTypesProportions(edgetype1, edgetype2, correlation);
            updateParametersAverage(parameters, averageParameters, i-burnin+1);

            if (writeSamplesToFile)
                writeStateToFile(i-burnin);
        }
        outputProgressToConsole(i+1, sampleSize, burnin);
    }
    return {getMostCommonEdgeTypes(edgetype1, edgetype2, sampleSize), averageParameters};
}

std::pair<EdgeTypeFrequencies, EdgeTypeFrequencies> GibbsBase::sampleAndGetOccurences(size_t sampleSize, size_t burnin, bool correlation, bool writeSamplesToFile) {
    resetValues();
    outputProgressToConsole(0, sampleSize, burnin);

    // No edge (type 0) is deduced from the others.
    EdgeTypeFrequencies edgetype1(hypergraph.getSize());
    EdgeTypeFrequencies edgetype2(hypergraph.getSize());

    for (size_t i=0; i<sampleSize+burnin; i++) {
        sampleFromPosterior();

        if (i >= burnin) {
            updateTypesProportions(edgetype1, edgetype2, correlation);

            if (writeSamplesToFile)
                writeStateToFile(i-burnin);
        }
        outputProgressToConsole(i+1, sampleSize, burnin);
    }
    return {edgetype1, edgetype2};
}

void GibbsBase::updateTypesProportions(EdgeTypeFrequencies& edgetype1, EdgeTypeFrequencies& edgetype2, bool correlation) const {
    size_t currentEdgeType;

    for (size_t i=0; i<hypergraph.getSize(); i++)
        for (size_t j=i+1; j<hypergraph.getSize(); j++) {
            if (correlation)
                currentEdgeType = hypergraph.getHighestOrderHyperedgeWith(i, j);
            else
                currentEdgeType = hypergraph.getEdgeMultiplicity(i, j);

            if (currentEdgeType == 0)
                continue;

            auto& edgeMapToUpdate = currentEdgeType==1 ? edgetype1 : edgetype2;

            if (edgeMapToUpdate[i].find(j) == edgeMapToUpdate[i].end())
                edgeMapToUpdate[i][j] = 1;
            else
                edgeMapToUpdate[i][j]++;
        }
}

static size_t resolve3WayTie(const std::array<size_t, 3>& values) {
    return values[std::uniform_int_distribution<size_t>(0, 2)(generator)];
}

static size_t resolve2WayTie(const std::array<size_t, 2>& values) {
    return values[std::uniform_int_distribution<size_t>(0, 1)(generator)];
}

static inline size_t getOccurencesOf(const EdgeTypeFrequencies& edgetype, size_t i, size_t j) {
    const auto it = edgetype[i].find(j);
    if (it != edgetype[i].end())
        return it->second;
    return 0;
}

static size_t getMostCommonType(const std::array<size_t, 3>& occurences) {
    size_t mostCommonType = 0;

    if (occurences[0] > occurences[1]) {
        if (occurences[0] > occurences[2])
            mostCommonType = 0;
        else if(occurences[0] < occurences[2])
            mostCommonType = 2;
        else
            mostCommonType = resolve2WayTie({0, 2});
    }
    else if (occurences[1] > occurences[0]) {
        if (occurences[1] > occurences[2])
            mostCommonType = 1;
        else if (occurences[1] < occurences[2])
            mostCommonType = 2;
        else
            mostCommonType = resolve2WayTie({1, 2});
    }
    else {
        if (occurences[1] > occurences[2])
            mostCommonType = resolve2WayTie({0, 1});
        else if (occurences[1] < occurences[2])
            mostCommonType = 2;
        else
            resolve3WayTie({0, 1, 2});
    }
    return mostCommonType;
}

Hypergraph GibbsBase::getMostCommonEdgeTypes(EdgeTypeFrequencies& edgetype1, EdgeTypeFrequencies& edgetype2, size_t sampleSize) const {
    Hypergraph mostCommonEdgeTypes(hypergraph.getSize());
    std::array<size_t, 3> occurences;

    for (size_t i=0; i<hypergraph.getSize(); i++)
        for (size_t j=i+1; j<hypergraph.getSize(); j++) {
            occurences[1] = getOccurencesOf(edgetype1, i, j);
            occurences[2] = getOccurencesOf(edgetype2, i, j);
            occurences[0] = sampleSize - occurences[1] - occurences[2];

            mostCommonEdgeTypes.addMultiedge(i, j, getMostCommonType(occurences));
        }

    return mostCommonEdgeTypes;
}

static size_t getPercentProgress(size_t i, size_t n) {
    return (i*100)/n;
}

void GibbsBase::outputProgressToConsole(size_t iteration, size_t sampleSize, size_t burnin) const{
    if (verbose < 1)
        return;

    bool inBurnin = iteration < burnin;

    size_t actualIteration = inBurnin ? iteration : iteration-burnin;
    size_t actualSampleSize = inBurnin ? burnin : sampleSize;


    if (actualIteration < actualSampleSize) {
        size_t currentProgress = getPercentProgress(actualIteration, actualSampleSize);

        // print only when value changed
        if (actualIteration == 0 || getPercentProgress(actualIteration-1, actualSampleSize) != currentProgress) {
            printf("Chain %lu (%s): %lu%%   \r", chainID+1, inBurnin ? "Burn-in" : "Sampling", currentProgress);
            fflush(stdout);
        }
    }
    else if (actualIteration == actualSampleSize) { // always false in burn-in
        printf("Chain %lu Done                 \n", chainID+1);
        fflush(stdout);
    }
}

} //namespace GRIT
