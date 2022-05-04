#include "GRIT/parameters-samplers/poisson_gilbert.h"
#include <boost/math/special_functions/gamma.hpp>


using namespace std;


namespace GRIT{

void PoissonGilbertParametersSampler::sample() {
    // X0, X1
    auto occ = countOccurences();

    size_t edgeNumber = hypergraph.getEdgeNumber();

    parameters[0] = drawFromBeta(edgeNumber+hyperParameters[0], nchoose2-edgeNumber+hyperParameters[1]);
    parameters[1] = 0;
    parameters[2] = drawFromUpperTruncatedGammaITS(parameters[3], occ[0]+hyperParameters[4], 1/(nchoose2-edgeNumber+hyperParameters[5]));
    parameters[3] = drawFromTruncatedGamma(parameters[2], MEAN_MAX, occ[1]+hyperParameters[6], 1/(edgeNumber+hyperParameters[7]));
    parameters[4] = 0;
}

vector<size_t> PoissonGilbertParametersSampler::countOccurences() const {
    // Index: 1=edge; 0=nothing
    size_t Xtilde[2] = {0, 0};


    bool isEdge;
    for (size_t i=0; i<hypergraph.getSize(); i++) {
        for (size_t j=i+1; j<hypergraph.getSize(); j++) {

            isEdge = hypergraph.isEdge(i, j);
            Xtilde[isEdge] += observations[i][j];
        }
    }

    return {Xtilde[0], Xtilde[1]};
}


}// namespace GRIT
