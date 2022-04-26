#ifndef GRIT_BASE_MODEL_H
#define GRIT_BASE_MODEL_H

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"


class InferenceModel {
    public:
        double sample(size_t sampleSize, size_t burnin, size_t chain,
                    GRIT::Hypergraph& hypergraph, GRIT::Parameters& parameters, const GRIT::Observations& observations, const std::string& outputDirectory) const {
            try {
                return execute("sample", sampleSize, burnin, chain, 0, {}, hypergraph, parameters, observations, outputDirectory);
            }
            catch (std::runtime_error& err) {
                fprintf(stderr, "At throw: Parameters are [%E, %E, %E, %E, %E]. Hypergraph has %lu edges and %lu triangles.\n",
                                parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], hypergraph.getEdgeNumber(), hypergraph.getTriangleNumber());
                throw err;
            }
        }
        double sampleHypergraphs(size_t mhSteps, size_t points, const std::list<size_t>& iterations,
                    GRIT::Hypergraph& hypergraph, GRIT::Parameters& parameters, const GRIT::Observations& observations, const std::string& outputDirectory) const {
            return execute("sample_hypergraphs", mhSteps, 0, 0, points, iterations, hypergraph, parameters, observations, outputDirectory);
        }

        virtual double getLogLikelihood(const GRIT::Hypergraph& hypergraph, const GRIT::Parameters& parameters, const GRIT::Observations& observations) const = 0;
        virtual std::list<double> getPairwiseObservationsProbabilities(const GRIT::Hypergraph& hypergraph, const GRIT::Parameters& parameters, const GRIT::Observations& observations) const = 0;
        virtual GRIT::Observations generateObservations(const GRIT::Hypergraph&, const GRIT::Parameters&) const = 0;

    private:
        virtual double execute(const std::string& what, size_t sampleSize, size_t burnin, size_t chain, size_t points, const std::list<size_t>& iterations,
                    GRIT::Hypergraph&, GRIT::Parameters&, const GRIT::Observations&,
                    const std::string& outputDirectory) const = 0;
};

#endif
