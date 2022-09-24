#include <gtest/gtest.h>

#include "GRIT/utility.h"
#include "GRIT/hypergraph-models/edgestrength.h"
#include "GRIT/hypergraph-models/independent_hyperedges.h"


using namespace std;
using namespace GRIT;


static double p = 0.2;
static double q = 0.8;
static double& q1 = p;
static double& q2 = q;

static double mu0 = 1.5;
static double mu1 = 3.5;
static double mu2 = 5.5;
static Parameters parameters {p, q, mu0, mu1, mu2};


class HypergraphTestCase: public::testing::Test{
    public:
        GRIT::Hypergraph graph;
        Observations observations;

        HypergraphTestCase(): graph(5) {};

        void SetUp(){
            graph.addTriangle({0, 1, 3});
            graph.addTriangle({1, 2, 3});
            graph.addEdge(0, 1);
            graph.addEdge(0, 4);

            observations.resize(5, std::vector<size_t>(5, 0));
            size_t globalCount=0;
            for (auto i=0; i<5; i++)
                for (auto j=i+1; j<5; j++) {
                    observations[i][j] = globalCount;
                    observations[j][i] = globalCount;
                    globalCount++;
                }
        }
};

class EdgeStrengthGraphTestCase: public::testing::Test{
    public:
        GRIT::Hypergraph graph;
        Observations observations;

        EdgeStrengthGraphTestCase(): graph(3) {};

        void SetUp(){
            graph.addMultiedge(0, 1, 2);
            graph.addEdge(0, 2);

            observations.resize(3, std::vector<size_t>(3, 0));
            size_t globalCount=0;
            for (auto i=0; i<3; i++)
                for (auto j=i+1; j<3; j++) {
                    observations[i][j] = globalCount;
                    observations[j][i] = globalCount;
                    globalCount++;
                }
        }
};


TEST_F(HypergraphTestCase, independentHyperedges_when_addTriangle) {
    IndependentHyperedgesModel graphModel(graph, parameters, observations);
    EXPECT_DOUBLE_EQ(graphModel({ ADD, FourStepsHypergraphProposal::TRIANGLE, 0, 1, 2 }), log(p)-log(1-p));
    EXPECT_DOUBLE_EQ(graphModel(
      SixStepsHypergraphProposal{ ADD, SixStepsHypergraphProposal::TRIANGLE, Triplet{0, 1, 2},
                                  std::set<Edge>{{0, 1}, {0, 2}, {1, 2}}, 0}),
                     log(p)-log(1-p));
}

TEST_F(HypergraphTestCase, independentHyperedges_when_removeTriangle) {
    IndependentHyperedgesModel graphModel(graph, parameters, observations);
    EXPECT_DOUBLE_EQ(graphModel({ REMOVE, FourStepsHypergraphProposal::TRIANGLE, 0, 1, 3 }), log(1-p)-log(p));
    EXPECT_DOUBLE_EQ(graphModel(
      SixStepsHypergraphProposal{ REMOVE, SixStepsHypergraphProposal::TRIANGLE, Triplet{0, 1, 3},
                                  std::set<Edge>{{0, 1}, {0, 3}, {1, 3}}, 0}),
                     log(1-p)-log(p));
}

TEST_F(HypergraphTestCase, independentHyperedges_when_addEdge) {
    IndependentHyperedgesModel graphModel(graph, parameters, observations);
    EXPECT_DOUBLE_EQ(graphModel({ ADD, FourStepsHypergraphProposal::EDGE, 0, 2, -1 }), log(q)-log(1-q));
    EXPECT_DOUBLE_EQ(graphModel(
      SixStepsHypergraphProposal{ ADD, SixStepsHypergraphProposal::EDGE, Triplet{0, 2, 0}, std::set<Edge>{{0, 2}}}),
                     log(q)-log(1-q));
}

TEST_F(HypergraphTestCase, independentHyperedges_when_removeEdge) {
    IndependentHyperedgesModel graphModel(graph, parameters, observations);
    EXPECT_DOUBLE_EQ(graphModel({ REMOVE, FourStepsHypergraphProposal::EDGE, 0, 1, -1 }), log(1-q)-log(q));
    EXPECT_DOUBLE_EQ(graphModel(
      SixStepsHypergraphProposal{ REMOVE, SixStepsHypergraphProposal::EDGE, Triplet{0, 1, 0}, std::set<Edge>{{0, 1}}}),
                     log(1-q)-log(q));
}

TEST_F(HypergraphTestCase, independentHyperedges_when_cleanTriangles) {
    IndependentHyperedgesModel graphModel(graph, parameters, observations);
    EXPECT_DOUBLE_EQ(graphModel({ REMOVE, FourStepsHypergraphProposal::EDGE, 0, 1, -1 }), log(1-q)-log(q));
    EXPECT_DOUBLE_EQ(graphModel(
      SixStepsHypergraphProposal{ REMOVE, SixStepsHypergraphProposal::EDGE, Triplet{0, 1, 0}, std::set<Edge>{{0, 1}}}),
                     log(1-q)-log(q));
}


TEST_F(EdgeStrengthGraphTestCase, edgeStrength_when_addEdge) {
    EdgeStrengthGraphModel graphModel(graph, parameters, observations);
    EXPECT_THROW(graphModel({ ADD, {0, 1} }), std::logic_error);
    EXPECT_DOUBLE_EQ(graphModel({ ADD, {0, 2} }), log(q2)-log(1-q2)-log(q1));
    EXPECT_DOUBLE_EQ(graphModel({ ADD, {1, 2} }), log(q1)-log(1-q1));
}

TEST_F(EdgeStrengthGraphTestCase, edgeStrength_when_removeEdge) {
    EdgeStrengthGraphModel graphModel(graph, parameters, observations);
    EXPECT_DOUBLE_EQ(graphModel({ REMOVE, {0, 1} }), log(q1)+log(1-q2)-log(q2));
    EXPECT_DOUBLE_EQ(graphModel({ REMOVE, {0, 2} }), log(1-q1)-log(q1));
    EXPECT_THROW(graphModel({ REMOVE, {1, 2} }), std::logic_error);
}
