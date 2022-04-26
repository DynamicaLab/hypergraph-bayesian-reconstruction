#ifndef GRIT_GIBBS_BASE_H
#define GRIT_GIBBS_BASE_H


#include <string>
#include <functional>
#include <map>

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"


namespace GRIT {

typedef SparseMatrix<size_t> EdgeTypeFrequencies;
struct RandomVariables {
    GRIT::Hypergraph hypergraph;
    GRIT::Parameters parameters;
};


class GibbsBase {
    public:
        const std::string hypergraphSamplePrefix = "hypergraph";
        const std::string parametersSamplePrefix = "parameters";
        std::string hypergraphSampleDirectory = "./hypergraphsample/";
        std::string parameterSampleDirectory = "./parametersample/";

        size_t chainID=0;

    public:
        explicit GibbsBase(Hypergraph& hypergraph, Parameters& parameters, size_t verbose=2): hypergraph(hypergraph), parameters(parameters), verbose(verbose) {};
        virtual ~GibbsBase() {};

        virtual void sampleFromPosterior() = 0;
        virtual void sampleHypergraphChain(size_t mhSteps, size_t points, const std::list<size_t>& iterations) = 0;
        virtual double getAverageLogLikelihood() = 0;
        virtual void resetValues() = 0;

        void sample(size_t sampleSize, size_t burnin);
        RandomVariables sampleAndGetAverage(size_t sampleSize, size_t burnin, bool correlation=true, bool writeSamplesToFile=false);
        std::pair<EdgeTypeFrequencies, EdgeTypeFrequencies> sampleAndGetOccurences(size_t sampleSize, size_t burnin, bool correlation=true, bool writeSamplesToFile=false);
        template<typename T>
        std::vector<std::vector<T>> sampleCurrentChainWithMetrics(const std::list<std::function<T(const Hypergraph&, const Parameters&, const Observations&)>>& metrics,
                const std::function<Observations(const Hypergraph&, const Parameters&)>& observationsGeneratingFunction, size_t sampleSize, size_t burnin, bool writeSamplesToFile=false);

        void setVerbose(size_t v) { verbose=v; }

        void writeStateToFile(size_t iteration) const;
        void writeGraphStateToBinary(size_t iteration) const;
        void writeParametersStateToBinary(size_t iteration) const;

    protected:
        Hypergraph& hypergraph;
        Parameters& parameters;
        size_t verbose;

    protected:
        void outputProgressToConsole(size_t iteration, size_t sampleSize, size_t burnin) const;

    public: // public for testing
        void updateTypesProportions(EdgeTypeFrequencies& edgetype1, EdgeTypeFrequencies& edgetype2, bool correlated) const;
        Hypergraph getMostCommonEdgeTypes(EdgeTypeFrequencies& edgetype1, EdgeTypeFrequencies& edgetype2, size_t sampleSize) const;
};

template<typename T>
std::vector<std::vector<T>> GibbsBase::sampleCurrentChainWithMetrics(const std::list<std::function<T(const Hypergraph&, const Parameters&, const Observations&)>>& functions,
        const std::function<Observations(const Hypergraph&, const Parameters&)>& observationsGeneratingFunction, size_t sampleSize, size_t burnin, bool writeSamplesToFile) {

    if (functions.size() == 0)
        fprintf(stderr, "Warning: called method to sample with metrics but there are no metrics. The method \"sampling\" is more efficient for this task.\n");

    std::vector<std::vector<T>> values(functions.size());

    resetValues();
    outputProgressToConsole(0, sampleSize, burnin);  // Display process started
    for (size_t i=0; i<sampleSize+burnin; i++) {

        sampleFromPosterior();

        if (i >= burnin) {
            Observations generatedObservations = observationsGeneratingFunction(hypergraph, parameters);
            size_t j=0;
            for (const auto& func: functions) {
                values[j].push_back(func(hypergraph, parameters, generatedObservations));
                j++;
            }
            if (writeSamplesToFile)
                writeStateToFile(i-burnin);
        }
        outputProgressToConsole(i+1, sampleSize, burnin);
    }
    return values;
}

} //namespace GRIT

#endif
