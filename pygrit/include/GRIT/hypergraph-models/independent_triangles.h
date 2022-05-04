#ifndef GRIT_INDEPENDENT_TRIANGLES_MODEL_H
#define GRIT_INDEPENDENT_TRIANGLES_MODEL_H


#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"
#include "GRIT/proposers/movetypes.h"


namespace GRIT {

class IndependentTrianglesModel {
    const Hypergraph& hypergraph;
    const Parameters& parameters;
    const size_t pIndex = 0;

    public:
        IndependentTrianglesModel(const Hypergraph& hypergraph, const Parameters& parameters, const Observations& observations):
            hypergraph(hypergraph), parameters(parameters) {}

        double operator()(const Triplet& triplet, const AddRemoveMove& move) const;
        double getLoglikelihood() const {
            const double& p = parameters[pIndex];
            const size_t& n = hypergraph.getSize();
            const size_t& T = hypergraph.getTriangleNumber();
            return T*log(p) + ( nchoose3(n)-T )*log(1-p);
        }
};

} //namespace GRIT

#endif
