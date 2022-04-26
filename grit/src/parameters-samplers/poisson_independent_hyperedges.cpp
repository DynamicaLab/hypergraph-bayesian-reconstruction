#include "GRIT/parameters-samplers/poisson_independent_hyperedges.h"
#include <boost/math/special_functions/gamma.hpp>


using namespace std;


namespace GRIT{

void PoissonIndependentHyperedgesParameterSampler::sample() {
    // X0, X1, X2, A0, A1, A2
    auto occ = countOccurences();

    size_t triangleNumber = hypergraph.getTriangleNumber();
    size_t edgeNumber = hypergraph.getEdgeNumber();

    parameters[0] = drawFromBeta(triangleNumber+hyperParameters[0], nchoose3Value-triangleNumber+hyperParameters[1]);
    parameters[1] = drawFromBeta(edgeNumber+hyperParameters[2], nchoose2-edgeNumber+hyperParameters[3]);

    double noEdgeUpperBound = parameters[3]<parameters[4] ? parameters[3] : parameters[4];
    parameters[2] = drawFromTruncatedGamma(MEAN_MIN, noEdgeUpperBound, occ[0]+hyperParameters[4], 1/(occ[3]+hyperParameters[5]));
    parameters[3] = drawFromTruncatedGamma(parameters[2], MEAN_MAX, occ[1]+hyperParameters[6], 1/(occ[4]+hyperParameters[7]));
    parameters[4] = drawFromTruncatedGamma(parameters[2], MEAN_MAX, occ[2]+hyperParameters[8], 1/(occ[5]+hyperParameters[9]));
}

vector<size_t> PoissonIndependentHyperedgesParameterSampler::countOccurences() const {
    // Index: 2=triangle; 1=edge; 0=nothing
    size_t Xtilde[3] = {0, 0, 0};
    size_t Atilde[3] = {0, 0, 0};

    size_t hyperedgeType;
    for (size_t i=0; i<hypergraph.getSize(); i++) {
        for (size_t j=i+1; j<hypergraph.getSize(); j++) {
            hyperedgeType = hypergraph.getHighestOrderHyperedgeWith(i, j);

            Xtilde[hyperedgeType] += observations[i][j];
            Atilde[hyperedgeType] ++;
        }
    }

    vector<size_t> allvalues = {Xtilde[0], Xtilde[1], Xtilde[2],
                                Atilde[0], Atilde[1], Atilde[2]};

    return allvalues;
}

}// namespace GRIT
