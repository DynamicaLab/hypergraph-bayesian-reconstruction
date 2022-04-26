#include "GRIT/hypergraph-models/independent_triangles.h"


namespace GRIT {

double IndependentTrianglesModel::operator()(const Triplet& triplet, const AddRemoveMove& move) const{
    const double& p = parameters[pIndex];
    double logAcceptance = 0;

    if (move == ADD)
        logAcceptance += log(p) - log(1-p);
    else
        logAcceptance += -log(p) + log(1-p);
    return logAcceptance;
}

} //namespace GRIT
