#ifndef GRIT_METROPOLIS_HASTINGS_HPP
#define GRIT_METROPOLIS_HASTINGS_HPP

#include <stdexcept>
#include <iostream>
#include <array>

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"
#include "GRIT/proposers/proposer_base.h"


namespace GRIT {

template<typename Proposer, typename ObservationsModel, typename HypergraphModel, typename Prior=NOPRIOR>
class MetropolisHastings {

    Proposer& proposer;
    const ObservationsModel observationsModel;
    const HypergraphModel hypergraphModel;
    const Prior modelPriors;

    const size_t minIterations = 0;
    const size_t maxIterations = 50000;
    size_t chainLength = 0;
    size_t windowSize = 20000;
    double tolerance = 1e-3;

    double likelihoodAdjustment = 0;
    double previousLogLikelihoodAverage = 0;
    double currentLogLikelihood = 0;
    double averageLogLikelihood = 0;

    public:
        MetropolisHastings(Hypergraph& hypergraph, const Observations& observations, const Parameters& parameters, const Parameters& hyperparameters, Proposer& proposer,
                                const std::array<size_t, 2>& steps, size_t windowSize=20000, double tolerance=1e-3);
        void sample();
        void advanceOneStep();
        double getCurrentLoglikelihood() const { return currentLogLikelihood; }
        double getAverageLoglikelihood() const { return averageLogLikelihood; }
        double evaluateLogLikelihood() const;

        void resetValues();
        void recomputeProposersDistributions() { proposer.recomputeProposersDistributions(); }

        void processIteration(size_t iteration) {
            if (iteration % windowSize == 0) {
                if (iteration>0)
                    previousLogLikelihoodAverage = averageLogLikelihood;
                resetLikelihoodAverage();
            }
        }

        double getDistancePreviousAverage() const;

    private:
        bool hasConverged() const;
        double getLogAcceptanceProbability();
        void resetLikelihoodAverage() { chainLength = 0; averageLogLikelihood = 0; }

        bool acceptStep(double logAcceptance) const;
};


template<typename Proposer, typename T_observations, typename HypergraphModel, typename Prior>
MetropolisHastings<Proposer, T_observations, HypergraphModel, Prior>::MetropolisHastings(Hypergraph& hypergraph, const Observations& observations,
                                    const Parameters& parameters, const Parameters& hyperparameters, Proposer& proposer, const std::array<size_t, 2>& steps, size_t windowSize, double tolerance):
        proposer(proposer),
        observationsModel(hypergraph, parameters, observations), hypergraphModel(hypergraph, parameters, observations), modelPriors(parameters, hyperparameters),
        minIterations(steps[0]), maxIterations(steps[1]), windowSize(windowSize), tolerance(tolerance)
{
    chainLength = 0;
    likelihoodAdjustment = 0;
    averageLogLikelihood = 0;
    currentLogLikelihood = 0; // avoid computation that will be done later
}

template<typename Proposer, typename T_observations, typename HypergraphModel, typename Prior>
void MetropolisHastings<Proposer, T_observations, HypergraphModel, Prior>::resetValues() {
    chainLength = 0;
    likelihoodAdjustment = 0;
    averageLogLikelihood = 0;
    currentLogLikelihood = evaluateLogLikelihood();
}

template<typename Proposer, typename T_observations, typename HypergraphModel, typename Prior>
double MetropolisHastings<Proposer, T_observations, HypergraphModel, Prior>::evaluateLogLikelihood() const {
    double logLikelihood = 0;

    logLikelihood += observationsModel.getLoglikelihood();
    if (!std::isfinite(logLikelihood))
        throw std::runtime_error("MetropolisHastings: Observations model likelihood is NaN/inf.");

    logLikelihood += hypergraphModel.getLoglikelihood();
    if (!std::isfinite(logLikelihood))
        throw std::runtime_error("MetropolisHastings: Hypergraph model likelihood is NaN/inf.");

    logLikelihood += modelPriors();
    if (!std::isfinite(logLikelihood))
        throw std::runtime_error("MetropolisHastings: Prior evaluate to NaN/inf.");

    return logLikelihood;
}

template<typename Proposer, typename T_observations, typename HypergraphModel, typename Prior>
void MetropolisHastings<Proposer, T_observations, HypergraphModel, Prior>::sample() {
    resetValues();

    for (size_t i=0; i<maxIterations; i++) {
        if (i % windowSize == 0) {
            if (i>0)
                previousLogLikelihoodAverage = averageLogLikelihood;
            resetLikelihoodAverage();
        }
        advanceOneStep();

        if (i > minIterations && hasConverged())
            return;
    }
    std::cerr << "Warning: hypergraph chain has reached last iteration before converging" << std::endl;
}

template<typename Proposer, typename T_observations, typename HypergraphModel, typename Prior>
void MetropolisHastings<Proposer, T_observations, HypergraphModel, Prior>::advanceOneStep() {
    proposer.generateProposal();

    bool accept = acceptStep(getLogAcceptanceProbability());

    if (accept) {
        bool hypergraphChanged = proposer.applyStep();
        if (hypergraphChanged)
            currentLogLikelihood += likelihoodAdjustment;
    }

    chainLength++;  // Must be increased before the correction of the average
    averageLogLikelihood += (currentLogLikelihood-averageLogLikelihood) / chainLength;
}

template<typename Proposer, typename T_observations, typename HypergraphModel, typename Prior>
bool MetropolisHastings<Proposer, T_observations, HypergraphModel, Prior>::acceptStep(double logAcceptance) const {
    double draw = std::uniform_real_distribution<double>(0, 1)(generator);
    return draw <= exp(logAcceptance);
}

template<typename Proposer, typename T_observations, typename HypergraphModel, typename Prior>
double MetropolisHastings<Proposer, T_observations, HypergraphModel, Prior>::getLogAcceptanceProbability() {

    double logAcceptance = 0;

    logAcceptance += observationsModel(proposer.currentProposal);
    if (std::isnan(logAcceptance))
        throw std::runtime_error("MetropolisHastings: Observations model probability ratio is NaN.");

    logAcceptance += hypergraphModel(proposer.currentProposal);
    if (std::isnan(logAcceptance))
        throw std::runtime_error("MetropolisHastings: Hypergraph model probability ratio is NaN.");

    likelihoodAdjustment = logAcceptance;

    logAcceptance += proposer.getLogAcceptanceContribution();
    if (std::isnan(logAcceptance))
        throw std::runtime_error("MetropolisHastings: Proposal probability ratio is NaN.");

    return logAcceptance;
}

template<typename Proposer, typename T_observations, typename HypergraphModel, typename Prior>
bool MetropolisHastings<Proposer, T_observations, HypergraphModel, Prior>::hasConverged() const {
    return getDistancePreviousAverage() < tolerance;
}

template<typename Proposer, typename T_observations, typename HypergraphModel, typename Prior>
double MetropolisHastings<Proposer, T_observations, HypergraphModel, Prior>::getDistancePreviousAverage() const {
    if (-1 <= previousLogLikelihoodAverage && previousLogLikelihoodAverage <= 1)
        return std::abs(previousLogLikelihoodAverage-currentLogLikelihood);
    else
        return std::abs(previousLogLikelihoodAverage-currentLogLikelihood)/previousLogLikelihoodAverage;
}

} //namespace GRIT

#endif
