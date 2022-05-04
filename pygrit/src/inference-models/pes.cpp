#include "GRIT/inference-models/pes.h"


double PES::execute(const std::string& what, size_t sampleSize, size_t burnin, size_t chain, size_t points, const std::list<size_t>& iterations,
        GRIT::Hypergraph& hypergraph, GRIT::Parameters& parameters, const GRIT::Observations& observations,
        const std::string& outputDirectory) const {

    EdgeAdder edgeAdder(observations, hypergraph);
    EdgeRemover edgeRemover(hypergraph);

    Proposer proposer(hypergraph, parameters,
            modelHyperparameters, observations,
            edgeAdder, edgeRemover, eta);

    HypergraphSampler hypergraphSampler(hypergraph, observations, parameters,
            modelHyperparameters, proposer,
            {mhMinimumIterations, mhMaximumIterations},
            windowSize, tolerance);

    ParameterSampler parameterSampler = ParameterSampler(hypergraph, observations, parameters,
            modelHyperparameters);


    ModelSampler sampler(hypergraph, parameters, parameterSampler, hypergraphSampler);
    sampler.hypergraphSampleDirectory = outputDirectory;
    sampler.parameterSampleDirectory  = outputDirectory;
    sampler.chainID = chain;

    if (what == "sample") {
        auto edgeTypeOccurences = sampler.sampleAndGetOccurences(sampleSize, burnin, false, true);
        GRIT::writeSparseMatrixToBinary<size_t>(edgeTypeOccurences.first,  outputDirectory+"occurences"+std::to_string(chain)+"_edgetype1.bin");
        GRIT::writeSparseMatrixToBinary<size_t>(edgeTypeOccurences.second, outputDirectory+"occurences"+std::to_string(chain)+"_edgetype2.bin");
    }
    else if (what == "sample_hypergraphs")
        sampler.sampleHypergraphChain(sampleSize, points, iterations);

    return sampler.getAverageLogLikelihood();
}

GRIT::Observations PES::generateObservations(const GRIT::Hypergraph& hypergraph, const GRIT::Parameters& parameters) const {
    size_t size = hypergraph.getSize();
    GRIT::Observations observations(size, std::vector<size_t>(size, 0));

    std::poisson_distribution<size_t> distribution[3] = {
                            std::poisson_distribution<size_t>(parameters[2]),
                            std::poisson_distribution<size_t>(parameters[3]),
                            std::poisson_distribution<size_t>(parameters[4])};


    size_t observationsElement;
    for (size_t i=0; i<size; i++){
        for (size_t j=i+1; j<size; j++){
            const size_t& edgeMultiplicity = hypergraph.getEdgeMultiplicity(i, j);

            observationsElement = distribution[edgeMultiplicity](GRIT::generator);
            observations[i][j] = observationsElement;
            observations[j][i] = observationsElement;
        }
    }
    return observations;
}

std::list<double> PES::getPairwiseObservationsProbabilities(const GRIT::Hypergraph &hypergraph, const GRIT::Parameters &parameters, const GRIT::Observations &observations) const {
    std::list<double> probabilities;
    size_t n = hypergraph.getSize();

    for (size_t i=0; i<n; i++)
        for (size_t j=i+1; j<n; j++) {
            auto & mean = parameters[2+hypergraph.getEdgeMultiplicity(i, j)];
            probabilities.push_back( exp( observations[i][j]*log(mean) - lgamma(observations[i][j]+1) - mean ) );
        }
    return probabilities;
}
