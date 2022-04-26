#ifndef PES_H
#define PES_H

#include "GRIT/inference-models/base_model.h"

#include "GRIT/gibbs_sampler.hpp"
#include "GRIT/metropolis-hastings.hpp"

#include "GRIT/parameters-samplers/poisson_edgestrength.h"
#include "GRIT/observations-models/poisson_edgestrength.h"
#include "GRIT/hypergraph-models/edgestrength.h"
#include "GRIT/priors/poisson_independent_hyperedges.h"

#include "GRIT/proposers/twosteps_edges.h"
#include "GRIT/proposers/edge-choosers/uniform_edge_chooser.h"
#include "GRIT/proposers/edge-choosers/weighted_two-layers_chooser.h"

#include "GRIT/inference-models/model_likelihood.hpp"


class PES: public InferenceModel {
    typedef GRIT::TwoLayersObservationsWeightedEdgeChooser EdgeAdder;
    typedef GRIT::UniformNonEdgeChooser                    EdgeRemover;

    typedef GRIT::PoissonEdgeStrengthObservationsModel ObservationsModel;
    typedef GRIT::EdgeStrengthGraphModel               HypergraphModel;
    typedef GRIT::PoissonHypergraph_BetaAndGammaPriors Prior;
    typedef GRIT::EdgeTwoStepsProposer                 Proposer;

    typedef GRIT::MetropolisHastings<Proposer, ObservationsModel, HypergraphModel, Prior> HypergraphSampler;
    typedef GRIT::PoissonEdgeStrengthParametersSampler                                    ParameterSampler;
    typedef GRIT::GibbsSampler<ParameterSampler, HypergraphSampler>                       ModelSampler;


    size_t windowSize;
    double tolerance;
    size_t mhMinimumIterations, mhMaximumIterations;
    double eta;

    std::vector<double> modelHyperparameters, moveProbabilities;

    public:
        PES(size_t windowSize, double tolerance, size_t mhMinimumIterations,size_t mhMaximumIterations,
                double eta,
                const std::vector<double>& modelHyperparameters, const std::vector<double>& moveProbabilities
                ):
            windowSize(windowSize), tolerance(tolerance),
            mhMinimumIterations(mhMinimumIterations), mhMaximumIterations(mhMaximumIterations),
            eta(eta),
            modelHyperparameters(modelHyperparameters), moveProbabilities(moveProbabilities)
        {}

        double getLogLikelihood(const GRIT::Hypergraph& hypergraph, const GRIT::Parameters& parameters, const GRIT::Observations& observations) const override {
            return ::getLogLikelihood<ObservationsModel, HypergraphModel, Prior>(hypergraph, parameters, observations, modelHyperparameters);
        }
        std::list<double> getPairwiseObservationsProbabilities(const GRIT::Hypergraph& hypergraph, const GRIT::Parameters& parameters, const GRIT::Observations& observations) const override;

        GRIT::Observations generateObservations(const GRIT::Hypergraph&, const GRIT::Parameters&) const override;

    private:
        double execute(const std::string& what, size_t sampleSize, size_t burnin, size_t chain, size_t points, const std::list<size_t>& iterations,
                    GRIT::Hypergraph&, GRIT::Parameters&, const GRIT::Observations&,
                    const std::string& outputDirectory) const override;
};

#endif
