#include <gtest/gtest.h>

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"
#include "GRIT/gibbs_base.h"


class GibbsTesting: public GRIT::GibbsBase {
    public:
        explicit GibbsTesting(GRIT::Hypergraph& hypergraph, GRIT::Parameters& parameters, size_t verbose=2): GRIT::GibbsBase(hypergraph, parameters, verbose) {};

        void sampleFromPosterior() {}
        void sampleHypergraphChain(size_t mhSteps, size_t points, const std::list<size_t>& iterations) {}
        double getAverageLogLikelihood() {return 0;}
        void resetValues() {}
};

static inline size_t getOccurencesOf(const GRIT::EdgeTypeFrequencies& edgetype, size_t i, size_t j) {
    const auto it = edgetype[i].find(j);
    if (it != edgetype[i].end())
        return it->second;
    return 0;
}


TEST(updateMostCommonTypes, multigraph_correctUpdate) {
    GRIT::EdgeTypeFrequencies types[2];
    types[0].resize(5);
    types[1].resize(5);

    GRIT::Hypergraph sampledMultigraph(5);
    sampledMultigraph.addMultiedge(0, 1, 2);
    sampledMultigraph.addMultiedge(0, 3, 2);
    sampledMultigraph.addMultiedge(1, 3, 2);
    sampledMultigraph.addEdge(0, 2);
    sampledMultigraph.addEdge(2, 3);
    sampledMultigraph.addEdge(1, 4);

    GRIT::Parameters p;
    GibbsTesting gibbsTester(sampledMultigraph, p);

    gibbsTester.updateTypesProportions(types[0], types[1], false);


    GRIT::Observations averageHypergraphTypes = {
        {0, 2, 1, 2, 0},
        {0, 0, 0, 2, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0}
    };

    auto averageHypergraph = gibbsTester.getMostCommonEdgeTypes(types[0], types[1], 1);
    auto n = sampledMultigraph.getSize();

    for (size_t i=0; i<n; i++)
        for (size_t j=i+1; j<n; j++)
            EXPECT_EQ(averageHypergraph.getEdgeMultiplicity(i, j), averageHypergraphTypes[i][j]);
}


TEST(updateMostCommonTypes, hypergraph_correctUpdate) {
    GRIT::EdgeTypeFrequencies types[2];
    types[0].resize(5);
    types[1].resize(5);


    GRIT::Hypergraph sampledHypergraph(5);
    sampledHypergraph.addTriangle({0, 1, 3});
    sampledHypergraph.addEdge(0, 2);
    sampledHypergraph.addEdge(2, 3);
    sampledHypergraph.addEdge(1, 4);

    GRIT::Parameters p;
    GibbsTesting gibbsTester(sampledHypergraph, p);

    gibbsTester.updateTypesProportions(types[0], types[1], true);


    GRIT::Observations averageHypergraphTypes = {
        {0, 2, 1, 2, 0},
        {0, 0, 0, 2, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0}
    };

    auto averageHypergraph = gibbsTester.getMostCommonEdgeTypes(types[0], types[1], 1);
    auto n = sampledHypergraph.getSize();

    for (size_t i=0; i<n; i++)
        for (size_t j=i+1; j<n; j++)
            EXPECT_EQ(averageHypergraph.getEdgeMultiplicity(i, j), averageHypergraphTypes[i][j]);
}
