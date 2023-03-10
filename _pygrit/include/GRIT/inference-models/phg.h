#ifndef PHG_H
#define PHG_H

#include "GRIT/inference-models/base_model.h"

#include "GRIT/gibbs_sampler.hpp"
#include "GRIT/metropolis-hastings.hpp"

#include "GRIT/parameters-samplers/poisson_independent_hyperedges.h"
#include "GRIT/observations-models/poisson_hypergraph.h"
#include "GRIT/hypergraph-models/independent_hyperedges.h"
#include "GRIT/priors/poisson_independent_hyperedges.h"

#include "GRIT/proposers/sixsteps_hypergraph.h"
#include "GRIT/proposers/edge-choosers/uniform_edge_chooser.h"
#include "GRIT/proposers/edge-choosers/weighted_unique_chooser.h"
#include "GRIT/proposers/triangle-choosers/observations_by_pairs_chooser.h"
#include "GRIT/proposers/triangle-choosers/uniform_triangle_chooser.h"

#include "GRIT/inference-models/model_likelihood.hpp"


class PHG: public InferenceModel {
    typedef GRIT::ObservationsWeightedUniqueEdgeChooser EdgeAdder;
    typedef GRIT::UniformNonEdgeChooser                 EdgeRemover;
    typedef GRIT::ObservationsPairwiseTriangleChooser   TriangleAdder;
    typedef GRIT::UniformTriangleChooser                TriangleRemover;

    typedef GRIT::PoissonHypergraphObservationsModel    ObservationsModel;
    typedef GRIT::IndependentHyperedgesModel            HypergraphModel;
    typedef GRIT::PoissonHypergraph_BetaAndGammaPriors  Prior;
    typedef GRIT::HypergraphSixStepsProposer            Proposer;

    typedef GRIT::MetropolisHastings<Proposer, ObservationsModel, HypergraphModel, Prior> HypergraphSampler;
    typedef GRIT::PoissonIndependentHyperedgesParameterSampler                            ParameterSampler;
    typedef GRIT::GibbsSampler<ParameterSampler, HypergraphSampler>                       ModelSampler;


    size_t windowSize;
    double tolerance;
    size_t mhMinimumIterations, mhMaximumIterations;
    double eta, chi_0, chi_1;
    std::vector<double> modelHyperparameters, moveProbabilities;

    public:
        PHG(size_t windowSize, double tolerance, size_t mhMinimumIterations,size_t mhMaximumIterations,
                double eta, double chi_0, double chi_1,
                const std::vector<double>& modelHyperparameters, const std::vector<double>& moveProbabilities
                ):
            windowSize(windowSize), tolerance(tolerance),
            mhMinimumIterations(mhMinimumIterations), mhMaximumIterations(mhMaximumIterations),
            eta(eta), chi_0(chi_0), chi_1(chi_1),
            modelHyperparameters(modelHyperparameters), moveProbabilities(moveProbabilities)
        {}

        void setHyperparameters(const std::vector<double>& newHyperparameters) { modelHyperparameters = newHyperparameters; }

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
