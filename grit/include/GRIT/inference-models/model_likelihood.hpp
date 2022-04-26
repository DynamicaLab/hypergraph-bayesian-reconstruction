#ifndef INFERENCE_BASE_MODEL_HPP
#define INFERENCE_BASE_MODEL_HPP

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"


template<typename ObservationsModel, typename HypergraphModel, typename Prior>
double getLogLikelihood(const GRIT::Hypergraph& hypergraph, const GRIT::Parameters& parameters, const GRIT::Observations& observations, const GRIT::Parameters& hyperparameters) {
    ObservationsModel observationsModel(hypergraph, parameters, observations);
    HypergraphModel hypergraphModel(hypergraph, parameters, observations);
    Prior prior(parameters, hyperparameters);

    return observationsModel.getLoglikelihood() + hypergraphModel.getLoglikelihood() + prior();
}

#endif
