#include <gtest/gtest.h>

#include "GRIT/utility.h"
#include "GRIT/proposers/sixsteps_hypergraph.h"
#include "GRIT/proposers/edge-choosers/weighted_unique_chooser.h"
#include "GRIT/proposers/edge-choosers/uniform_edge_chooser.h"
#include "GRIT/proposers/triangle-choosers/observations_by_pairs_chooser.h"
#include "GRIT/proposers/triangle-choosers/uniform_triangle_chooser.h"


using namespace std;
using namespace GRIT;


static const double eta=0.3;
static const double chi_0=0.6;
static const double chi_1=0.3;
static const std::vector<double> moveProbabilities = {0.5, 0.3, 0.2};


// 3/8 hidden 2-edges in the hypergraph
class SixStepsHypergraph_testCase: public::testing::Test{
    public:
        GRIT::Hypergraph hypergraph;
        Observations observations;
        Parameters parameters;
        Parameters hyperParameters;

        SixStepsHypergraph_testCase(): hypergraph(8), parameters({1, 1, 2, 3}), hyperParameters({0.5, 0.5, 1, 2, 3, 4, 5, 6}) {};

        void SetUp(){
            hypergraph.addTriangle({0, 1, 2});
            hypergraph.addTriangle({0, 1, 3});
            hypergraph.addTriangle({3, 4, 5});
            hypergraph.addEdge(0, 1);
            hypergraph.addEdge(0, 2);
            hypergraph.addEdge(2, 3);
            hypergraph.addEdge(3, 4);

            observations = {{0, 1, 2, 0, 3, 1},
                    {1, 0, 0, 1, 2, 3},
                    {2, 0, 0, 1, 1, 4},
                    {0, 1, 1, 0, 0, 0},
                    {3, 2, 1, 0, 0, 1},
                    {1, 3, 4, 0, 1, 0}};
        }
};


TEST_F(SixStepsHypergraph_testCase, when_addingEdge_expect_correctProposalContribution) {

    ObservationsWeightedUniqueEdgeChooser edgeAdder(observations, hypergraph);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(observations);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, observations, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi_0, chi_1);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::EDGE;
    proposal.move = ADD;
    proposal.changedPairs = {{0, 3}};
    proposal.chosenTriplet = {0, 3, 0};

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(1-eta)-log(eta) + log(edgeRemover.getReverseProbability({0, 3}, ADD))
            - log(edgeAdder.getForwardProbability({0, 3}, ADD)),
            0.00001);
}

TEST(SixStepsHypergraph, when_proposingGraphFullOfEdges_expect_onewayEtaContribution) {
    Observations observations({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});

    Hypergraph hypergraph(4);
    for (size_t i=0; i<4; i++)
        for (size_t j=i+1; j<4; j++)
            if (!(i==0 && j==1))
                hypergraph.addEdge(i, j);

    ObservationsWeightedUniqueEdgeChooser edgeAdder(observations, hypergraph);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(observations);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, observations, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi_0, chi_1);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::EDGE;
    proposal.move = ADD;
    proposal.changedPairs = {{0, 1}};
    proposal.chosenTriplet = {0, 1, 0};

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
                log(edgeRemover.getReverseProbability({0, 1}, ADD))
                  - log(edgeAdder.getForwardProbability({0, 1}, ADD)) - log(eta),
                0.00001);
}


TEST_F(SixStepsHypergraph_testCase, when_removingEdge_expect_correctProposalContribution) {

    ObservationsWeightedUniqueEdgeChooser edgeAdder(observations, hypergraph);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(observations);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, observations, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi_0, chi_1);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::EDGE;
    proposal.move = REMOVE;
    proposal.changedPairs = {{0, 1}};
    proposal.chosenTriplet = {0, 1, 0};

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(eta)-log(1-eta) + log(edgeAdder.getReverseProbability({0, 1}, REMOVE))
                                                    - log(edgeRemover.getForwardProbability({0, 1}, REMOVE)),
            0.00001);
}

TEST(SixStepsHypergraph, when_proposingGraphWithoutEdges_expect_onewayEtaContribution) {
    Observations observations({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});

    Hypergraph hypergraph(4);
    hypergraph.addEdge(0, 1);

    ObservationsWeightedUniqueEdgeChooser edgeAdder(observations, hypergraph);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(observations);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, observations, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi_0, chi_1);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::EDGE;
    proposal.move = REMOVE;
    proposal.changedPairs = {{0, 1}};
    proposal.chosenTriplet = {0, 1, 0};

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
                log(edgeAdder.getReverseProbability({0, 1}, REMOVE))
                  - log(edgeRemover.getForwardProbability({0, 1}, REMOVE)) - log(1-eta),
                0.00001);
}


TEST_F(SixStepsHypergraph_testCase, when_addingTriangle_expect_correctProposalContribution) {
    ObservationsWeightedUniqueEdgeChooser edgeAdder(observations, hypergraph);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(observations);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, observations, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi_0, chi_1);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::TRIANGLE;
    proposal.move = ADD;
    proposal.changedPairs = {{0, 1}, {0, 2}, {1, 2}};
    proposal.chosenTriplet = {0, 1, 2};

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(1-eta)-log(eta) + log(triangleRemover.getReverseProbability({0, 1, 2}, ADD))
                - log(triangleAdder.getForwardProbability({0, 1, 2}, ADD)),
            0.00001);
}

TEST(SixStepsHypergraph, when_proposingHypergraphFullOfTriangles_expect_onewayEtaContribution) {
Observations observations({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});

    Hypergraph hypergraph(4);

    for (size_t i=0; i<4; i++)
        for (size_t j=i+1; j<4; j++)
            for (size_t k=j+1; k<4; k++)
                if (!(i==0 && j==1 && k==2))
                    hypergraph.addTriangle({i, j, k});

    ObservationsWeightedUniqueEdgeChooser edgeAdder(observations, hypergraph);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(observations);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, observations, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi_0, chi_1);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::TRIANGLE;
    proposal.move = ADD;
    proposal.changedPairs = {{0, 1}, {0, 2}, {1, 2}};
    proposal.chosenTriplet = {0, 1, 2};

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(triangleRemover.getReverseProbability({0, 1, 2}, ADD))
                - log(triangleAdder.getForwardProbability({0, 1, 2}, ADD)) - log(eta),
            0.00001);
}

TEST_F(SixStepsHypergraph_testCase, when_removingTriangle_expect_correctProposalContribution) {
    ObservationsWeightedUniqueEdgeChooser edgeAdder(observations, hypergraph);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(observations);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, observations, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi_0, chi_1);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::TRIANGLE;
    proposal.move = REMOVE;
    proposal.changedPairs = {{0, 1}, {0, 3}, {1, 3}};
    proposal.chosenTriplet = {0, 1, 3};

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(eta)-log(1-eta) + log(triangleAdder.getReverseProbability({0, 1, 3}, REMOVE))
                - log(triangleRemover.getForwardProbability({0, 1, 3}, REMOVE)),
            0.00001);
}

TEST(SixStepsHypergraph, when_proposingHypergraphEmptyOfTriangles_expect_onewayEtaContribution) {
    Observations observations({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});

    Hypergraph hypergraph(4);
    hypergraph.addTriangle({0, 1, 2});

    ObservationsWeightedUniqueEdgeChooser edgeAdder(observations, hypergraph);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(observations);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, observations, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi_0, chi_1);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::TRIANGLE;
    proposal.move = REMOVE;
    proposal.changedPairs = {{0, 1}, {0, 2}, {1, 2}};
    proposal.chosenTriplet = {0, 1, 2};

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(triangleAdder.getReverseProbability({0, 1, 2}, REMOVE))
                - log(triangleRemover.getForwardProbability({0, 1, 2}, REMOVE)) - log(1-eta),
            0.00001);
}


TEST_F(SixStepsHypergraph_testCase, when_cleaningTriangles_expect_correctProposalAndContribution) {
    ObservationsWeightedUniqueEdgeChooser edgeAdder(observations, hypergraph);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(observations);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, observations, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi_0, chi_1);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::HIDDEN_EDGES;
    proposal.move = REMOVE;
    proposal.unchangedPairsNumber = 0;

    generator.seed(100);
    proposer.setProposal(proposal);
    proposer.proposeHiddenEdges();

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            (2-2)*log(1-chi_1)+log(chi_1) - log(1-pow(1-chi_1, 7-1)) + lgamma(7-2+1) - lgamma(7+1)
            - (2-2)*log(1-chi_0)-log(chi_0) + log(1-pow(1-chi_0, 3-1)) - lgamma(3-2+1) + lgamma(3+1)
            + log(eta)-log(1-eta),
            0.00001);
}

TEST_F(SixStepsHypergraph_testCase, when_dirtyingTriangles_expect_correctProposalAndContribution) {
    ObservationsWeightedUniqueEdgeChooser edgeAdder(observations, hypergraph);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(observations);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, observations, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi_0, chi_1);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::HIDDEN_EDGES;
    proposal.move = ADD;
    proposal.unchangedPairsNumber = 0;

    proposer.setProposal(proposal);
    generator.seed(100);
    proposer.proposeHiddenEdges();

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            (4-2)*log(1-chi_0)+log(chi_0) - log(1-pow(1-chi_0, 7-1)) + lgamma(7-4+1) - lgamma(7+1)
            - (4-2)*log(1-chi_1)-log(chi_1) + log(1-pow(1-chi_1, 5-1)) - lgamma(5-4+1) + lgamma(5+1)
            + log(1-eta)-log(eta),
            0.00001);
}
