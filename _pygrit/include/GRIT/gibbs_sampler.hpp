#ifndef GRIT_GIBBS_SAMPLER_HPP
#define GRIT_GIBBS_SAMPLER_HPP


#include <string>
#include <fstream>
#include <stdexcept>
#include <set>

#include "GRIT/gibbs_base.h"
#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"


namespace GRIT {

template<typename T_parameterSampler, typename T_hypergraphSampler>
class GibbsSampler: public GibbsBase {
    T_parameterSampler& parameterSampler;
    T_hypergraphSampler& hypergraphSampler;

    size_t chainLength = 0;
    double currentLogLikelihood = 0;
    double averageLogLikelihood = 0;

    public:
        explicit GibbsSampler(Hypergraph&, Parameters&, T_parameterSampler&, T_hypergraphSampler&);

        //void executeBurninIteration() { sampleFromPosterior(); }
        void sampleFromPosterior();
        double getAverageLogLikelihood() { return averageLogLikelihood; }

        void sampleHypergraphChain(size_t mhSteps, size_t points, const std::list<size_t>& iterations);
        void resetValues() { chainLength=0, averageLogLikelihood=0; currentLogLikelihood=0; hypergraphSampler.recomputeProposersDistributions(); }

    private:
        void sampleParametersFromPosterior() { parameterSampler.sample(); }
        void sampleHypergraphFromPosterior() { hypergraphSampler.sample(); }
        void sampleAndWriteHypergraphChain(size_t mhSteps, size_t points, const std::string& outputDirectory);
};


template<typename T_parameterSampler, typename T_hypergraphSampler>
GibbsSampler<T_parameterSampler, T_hypergraphSampler>::GibbsSampler(
        Hypergraph& hypergraph, Parameters& parameters, T_parameterSampler& parameterSampler, T_hypergraphSampler& hypergraphSampler):
    GibbsBase(hypergraph, parameters),
    parameterSampler(parameterSampler),
    hypergraphSampler(hypergraphSampler)
{}


template<typename T_parameterSampler, typename T_hypergraphSampler>
void GibbsSampler<T_parameterSampler, T_hypergraphSampler>::sampleFromPosterior() {
    sampleHypergraphFromPosterior();
    sampleParametersFromPosterior();

    chainLength++;  // Has to be increased before the computation of the average
    currentLogLikelihood = hypergraphSampler.evaluateLogLikelihood();
    averageLogLikelihood += (currentLogLikelihood-averageLogLikelihood) / chainLength;
}

template<typename T_parameterSampler, typename T_hypergraphSampler>
void GibbsSampler<T_parameterSampler, T_hypergraphSampler>::sampleHypergraphChain(size_t mhSteps, size_t points, const std::list<size_t>& iterations) {
    resetValues();

    std::set<size_t> orderedIterations(iterations.begin(), iterations.end());

    size_t cumulativeIteration(0);
    for (auto iteration: orderedIterations) {
        for (size_t j=0; j<iteration-cumulativeIteration; j++) {
            sampleFromPosterior();
            cumulativeIteration++;
        }

        sampleAndWriteHypergraphChain(mhSteps, points, hypergraphSampleDirectory+std::to_string(cumulativeIteration)+"/");
        sampleParametersFromPosterior();
        chainLength++;  // Has to be increased before the computation of the average
        currentLogLikelihood = hypergraphSampler.evaluateLogLikelihood();
        averageLogLikelihood += (currentLogLikelihood-averageLogLikelihood) / chainLength;
        cumulativeIteration++;
    }

}

template<typename T_parameterSampler, typename T_hypergraphSampler>
void GibbsSampler<T_parameterSampler, T_hypergraphSampler>::sampleAndWriteHypergraphChain(size_t mhSteps, size_t points, const std::string& outputDirectory) {

    size_t skip = mhSteps/points;
    bool saveLastSample = ( (mhSteps%skip) != 0 );
    std::vector<double> likelihoods, distances;


    outputProgressToConsole(0, mhSteps, 0);

    hypergraphSampler.resetValues();
    for (size_t i=0; i<mhSteps; i++) {
        hypergraphSampler.advanceOneStep();
        hypergraphSampler.processIteration(i);
        if ( i%skip == 0 || (i == mhSteps-1 && saveLastSample) ) {
            likelihoods.push_back(hypergraphSampler.getCurrentLoglikelihood());
            distances.push_back(hypergraphSampler.getDistancePreviousAverage());

            hypergraph.writeToBinary(outputDirectory+hypergraphSamplePrefix+std::to_string(i)+".bin");
        }

        outputProgressToConsole(i+1, mhSteps, 0);
    }
    writeParametersToBinary(likelihoods, outputDirectory+"likelihood.bin");
    writeParametersToBinary(distances, outputDirectory+"distances.bin");
}

} //namespace GRIT

#endif
