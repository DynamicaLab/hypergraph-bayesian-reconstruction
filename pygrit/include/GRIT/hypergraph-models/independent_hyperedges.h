#ifndef GRIT_INDEPENDENT_HYPEREDGES_MODEL_H
#define GRIT_INDEPENDENT_HYPEREDGES_MODEL_H

#include <math.h>

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"
#include "GRIT/proposers/movetypes.h"


namespace GRIT {

class IndependentHyperedgesModel {
    const Hypergraph& hypergraph;
    const Parameters& parameters;
    const size_t pIndex = 0;
    const size_t qIndex = 1;

    public:
        IndependentHyperedgesModel(const Hypergraph& hypergraph, const Parameters& parameters, const Observations& observations):
            hypergraph(hypergraph), parameters(parameters) {}

        double operator()(const FourStepsHypergraphProposal& proposal) const;
        double operator()(const SixStepsHypergraphProposal& proposal) const;
        double getTriangleContribution(const AddRemoveMove& move) const;
        double getEdgeContribution(const AddRemoveMove& move) const;

        double getLoglikelihood() const {
            const double& p = parameters[pIndex];
            const double& q = parameters[qIndex];
            double logLikelihood = 0;

            const size_t& T = hypergraph.getTriangleNumber();
            const size_t& m = hypergraph.getEdgeNumber();
            const size_t& n = hypergraph.getSize();
            logLikelihood += T*log(p) + (nchoose3(n)-T)*log(1-p);
            logLikelihood += m*log(q) + (nchoose2(n)-m)*log(1-q);

            return logLikelihood;
        }
};

} //namespace GRIT

#endif
