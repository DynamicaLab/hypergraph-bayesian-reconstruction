#ifndef GRIT_MOVE_TYPES_H
#define GRIT_MOVE_TYPES_H

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"


namespace GRIT {

enum AddRemoveMove { REMOVE=0, ADD };

struct TwoStepsEdgeProposal{
    AddRemoveMove move;
    Edge chosenEdge;
    bool moveType;
};

struct TwoStepsTriangleProposal{
    AddRemoveMove move;
    Triplet chosenTriplet;
};


struct FourStepsHypergraphProposal{
    enum MoveType { NONE=0, EDGE, TRIANGLE };

    AddRemoveMove move;
    MoveType moveType;
    size_t i;
    size_t j;
    long long int k;
};

struct SixStepsHypergraphProposal{
    enum MoveType { NONE=0, EDGE, TRIANGLE, HIDDEN_EDGES };

    AddRemoveMove move;
    MoveType moveType;
    Triplet chosenTriplet;
    std::set<Edge> changedPairs;
    size_t unchangedPairsNumber;
    size_t maximumChangedPairsNumber;

    bool operator==(const SixStepsHypergraphProposal& other) const {
        return moveType == other.moveType && move == other.move && changedPairs == other.changedPairs;
    }
};

} //namespace GRIT

#endif
