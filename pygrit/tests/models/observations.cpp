#include <gtest/gtest.h>

#include "GRIT/utility.h"
#include "GRIT/observations-models/poisson_hypergraph.h"
#include "GRIT/observations-models/poisson_edgestrength.h"


using namespace std;
using namespace GRIT;


static double p = 0.5;
static double q = 0.5;
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


TEST_F(HypergraphTestCase, hyperedgeProposal_when_addTriangleWithEveryHyperedgeTypeUnder) {
    PoissonHypergraphObservationsModel observationsModel(graph, parameters, observations);

    FourStepsHypergraphProposal proposal { ADD, FourStepsHypergraphProposal::TRIANGLE, 0, 3, 4 };
    const size_t& ij = observations[proposal.i][proposal.j];
    const size_t& ik = observations[proposal.i][proposal.k];
    const size_t& jk = observations[proposal.j][proposal.k];

    EXPECT_DOUBLE_EQ(observationsModel(proposal), (double) ik*log(mu2/mu1) + (double) jk*log(mu2/mu0) - (mu2-mu1) - (mu2-mu0));
}

TEST_F(HypergraphTestCase, hyperedgeProposal_when_removeTriangleWithEveryHyperedgeTypeUnder) {
    PoissonHypergraphObservationsModel observationsModel(graph, parameters, observations);

    FourStepsHypergraphProposal proposal { REMOVE, FourStepsHypergraphProposal::TRIANGLE, 0, 3, 4 };
    const size_t& ij = observations[proposal.i][proposal.j];
    const size_t& ik = observations[proposal.i][proposal.k];
    const size_t& jk = observations[proposal.j][proposal.k];

    EXPECT_DOUBLE_EQ(observationsModel(proposal), (double) ik*log(mu1/mu2) + (double) jk*log(mu0/mu2) - (mu1-mu2) - (mu0-mu2));
}

TEST_F(HypergraphTestCase, hyperedgeProposal_when_addEdgeOverEveryHyperedgeType) {
    PoissonHypergraphObservationsModel observationsModel(graph, parameters, observations);

    const size_t i(0), j(3), k(4);
    const size_t& ij = observations[i][j];
    const size_t& ik = observations[i][k];
    const size_t& jk = observations[j][k];

    EXPECT_DOUBLE_EQ(observationsModel({ ADD, FourStepsHypergraphProposal::EDGE, i, j, -1 }), 0);
    EXPECT_DOUBLE_EQ(observationsModel({ ADD, FourStepsHypergraphProposal::EDGE, i, k, -1 }), 0);
    EXPECT_DOUBLE_EQ(observationsModel({ ADD, FourStepsHypergraphProposal::EDGE, j, k, -1 }), (double) jk*log(mu1/mu0) - (mu1-mu0));
}

TEST_F(HypergraphTestCase, hyperedgeProposal_when_removeEdgeCoveredOrNotByTriangle) {
    PoissonHypergraphObservationsModel observationsModel(graph, parameters, observations);

    const size_t i(0), j(3), k(4);
    const size_t& ij = observations[i][j];
    const size_t& ik = observations[i][k];
    const size_t& jk = observations[j][k];

    EXPECT_DOUBLE_EQ(observationsModel({ REMOVE, FourStepsHypergraphProposal::EDGE, i, j, -1 }), 0);
    EXPECT_DOUBLE_EQ(observationsModel({ REMOVE, FourStepsHypergraphProposal::EDGE, i, k, -1 }), (double) ik*log(mu0/mu1) - (mu0-mu1));

    // This is the expected behaviour, but the scenario should not happen (unless there's an edge with multiplicity 2) and the check would cost performance
    // EXPECT_DOUBLE_EQ(observationsModel({ REMOVE, FourStepsHypergraphProposal::EDGE, j, k, -1 }), 0);
}

TEST_F(EdgeStrengthGraphTestCase, edgeProposal_when_addEdgeOverEveryHyperedgeType) {
    PoissonEdgeStrengthObservationsModel observationsModel(graph, parameters, observations);

    const size_t i(0), j(1), k(2);
    const size_t& ij = observations[i][j];
    const size_t& ik = observations[i][k];
    const size_t& jk = observations[j][k];

    EXPECT_THROW(observationsModel({ ADD, {i, j} }), std::logic_error);
    EXPECT_DOUBLE_EQ(observationsModel({ ADD, {i, k} }), (double) ik*log(mu2/mu1) - (mu2-mu1));
    EXPECT_DOUBLE_EQ(observationsModel({ ADD, {j, k} }), (double) jk*log(mu1/mu0) - (mu1-mu0));
}

TEST_F(EdgeStrengthGraphTestCase, edgeProposal_when_removeEdgeOverEveryHyperedgeType) {
    PoissonEdgeStrengthObservationsModel observationsModel(graph, parameters, observations);

    const size_t i(0), j(1), k(2);
    const size_t& ij = observations[i][j];
    const size_t& ik = observations[i][k];
    const size_t& jk = observations[j][k];

    EXPECT_DOUBLE_EQ(observationsModel({ REMOVE, {i, j} }), (double) ij*log(mu1/mu2) - (mu1-mu2));
    EXPECT_DOUBLE_EQ(observationsModel({ REMOVE, {i, k} }), (double) ik*log(mu0/mu1) - (mu0-mu1));
    EXPECT_THROW(observationsModel({ REMOVE, {j, k} }), std::logic_error);
}
