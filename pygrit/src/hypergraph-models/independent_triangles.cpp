#include "GRIT/hypergraph-models/independent_triangles.h"


namespace GRIT {

double IndependentTrianglesModel::operator()(const Triplet& triplet, const AddRemoveMove& move) const{
    const double& p = parameters[pIndex];

    if (move == ADD)
        return log(p) - log(1-p);
    else
        return -log(p) + log(1-p);
}

} //namespace GRIT
