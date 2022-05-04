#include <gtest/gtest.h>

#include "GRIT/utility.h"
#include "GRIT/proposers/foursteps_hypergraph.h"
#include "GRIT/proposers/sixsteps_hypergraph.h"
#include "GRIT/proposers/edge-choosers/weighted_chooser.h"
#include "GRIT/proposers/edge-choosers/uniform_edge_chooser.h"
#include "GRIT/proposers/triangle-choosers/observations_by_pairs_chooser.h"
#include "GRIT/proposers/triangle-choosers/uniform_triangle_chooser.h"


using namespace std;
using namespace GRIT;


class FourStepsHypergraph_testCase: public::testing::Test{
    public:
        GRIT::Hypergraph hypergraph;
        Observations data;
        Parameters parameters;
        Parameters hyperParameters;
        double eta=0.3;
        double xi=0.4;

        FourStepsHypergraph_testCase(): hypergraph(4), parameters({1, 1, 2, 3}), hyperParameters({0.5, 0.5, 1, 2, 3, 4, 5, 6}) {};

        void SetUp(){
            hypergraph.addEdge(0, 1);
            hypergraph.addMultiedge(0, 2, 2);
            hypergraph.addEdge(1, 2);
            hypergraph.addTriangle({0, 1, 3});

            data = {{0, 1, 2, 0},
                    {1, 0, 0, 1},
                    {2, 0, 0, 1},
                    {0, 1, 1, 0} };
        }
};

class SixStepsHypergraph_testCase: public::testing::Test{
    public:
        GRIT::Hypergraph hypergraph;
        Observations data;
        Parameters parameters;
        Parameters hyperParameters;
        double eta=0.3;
        double chi=0.4;
        std::vector<double> moveProbabilities = {0.5, 0.3, 0.2};

        SixStepsHypergraph_testCase(): hypergraph(8), parameters({1, 1, 2, 3}), hyperParameters({0.5, 0.5, 1, 2, 3, 4, 5, 6}) {};

        void SetUp(){
            hypergraph.addTriangle({0, 1, 2});
            hypergraph.addTriangle({0, 1, 3});
            hypergraph.addTriangle({3, 4, 5});
            hypergraph.addEdge(0, 1);
            hypergraph.addEdge(0, 2);
            hypergraph.addEdge(2, 3);
            hypergraph.addEdge(3, 4);

            data = {{0, 1, 2, 0, 3, 1},
                    {1, 0, 0, 1, 2, 3},
                    {2, 0, 0, 1, 1, 4},
                    {0, 1, 1, 0, 0, 0},
                    {3, 2, 1, 0, 0, 1},
                    {1, 3, 4, 0, 1, 0}};
        }
};


TEST_F(FourStepsHypergraph_testCase, when_addingEdge_expect_correctProposalContribution) {

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphFourStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, eta, xi);

    FourStepsHypergraphProposal proposal;
    proposal.moveType = FourStepsHypergraphProposal::EDGE;
    proposal.move = ADD;
    proposal.i = 0;
    proposal.j = 3;
    proposal.k = -1;

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(1-eta)-log(eta) + log(edgeRemover.getReverseProbability({0, 3}, ADD))
            - log(edgeAdder.getForwardProbability({0, 3}, ADD)),
            0.00001);
}

TEST(FourStepsHypergraph, when_addingEdgeAndGraphHasNoEdge_expect_correctProposalContribution) {
    Observations data({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    double eta=0.3;
    double xi=0.4;
    Parameters parameters({1, 1, 1});

    Hypergraph hypergraph(4);

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphFourStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, eta, xi);

    FourStepsHypergraphProposal proposal;
    proposal.moveType = FourStepsHypergraphProposal::EDGE;
    proposal.move = ADD;
    proposal.i = 0;
    proposal.j = 1;
    proposal.k = -1;

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
                log(edgeRemover.getReverseProbability({0, 1}, ADD))
                  - log(edgeAdder.getForwardProbability({0, 1}, ADD)),
                0.00001);
}


TEST_F(FourStepsHypergraph_testCase, when_removingEdge_expect_correctProposalContribution) {

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphFourStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, eta, xi);

    FourStepsHypergraphProposal proposal;
    proposal.moveType = FourStepsHypergraphProposal::EDGE;
    proposal.move = REMOVE;
    proposal.i = 0;
    proposal.j = 1;
    proposal.k = -1;

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(eta)-log(1-eta) + log(edgeAdder.getReverseProbability({0, 1}, REMOVE))
                                                    - log(edgeRemover.getForwardProbability({0, 1}, REMOVE)),
            0.00001);
}

TEST_F(FourStepsHypergraph_testCase, when_addingTriangle_expect_correctProposalContribution) {

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphFourStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, eta, xi);

    FourStepsHypergraphProposal proposal;
    proposal.moveType = FourStepsHypergraphProposal::TRIANGLE;
    proposal.move = ADD;
    proposal.i = 0;
    proposal.j = 1;
    proposal.k = 2;

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(1-eta)-log(eta) + log(triangleRemover.getReverseProbability({0, 1, 2}, ADD))
                - log(triangleAdder.getForwardProbability({0, 1, 2}, ADD)),
            0.00001);
}

TEST(FourStepsHypergraph, when_addingTriangleAndGraphHasNoTriangle_expect_correctProposalContribution) {
Observations data({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    double eta=0.3;
    double xi=0.4;
    Parameters parameters({1, 1, 1});

    Hypergraph hypergraph(4);
    hypergraph.addEdge(0, 1);

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphFourStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, eta, xi);

    FourStepsHypergraphProposal proposal;
    proposal.moveType = FourStepsHypergraphProposal::TRIANGLE;
    proposal.move = ADD;
    proposal.i = 0;
    proposal.j = 1;
    proposal.k = 2;

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(triangleRemover.getReverseProbability({0, 1, 2}, ADD))
                - log(triangleAdder.getForwardProbability({0, 1, 2}, ADD)),
            0.00001);
}

TEST_F(FourStepsHypergraph_testCase, when_removingTriangle_expect_correctProposalContribution) {

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphFourStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, eta, xi);

    FourStepsHypergraphProposal proposal;
    proposal.moveType = FourStepsHypergraphProposal::TRIANGLE;
    proposal.move = REMOVE;
    proposal.i = 0;
    proposal.j = 1;
    proposal.k = 3;

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(eta)-log(1-eta) + log(triangleAdder.getReverseProbability({0, 1, 3}, REMOVE))
                - log(triangleRemover.getForwardProbability({0, 1, 3}, REMOVE)),
            0.00001);
}


TEST_F(SixStepsHypergraph_testCase, when_addingEdge_expect_correctProposalContribution) {

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi);

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

TEST(SixStepsHypergraph, when_addingEdgeAndGraphHasNoEdge_expect_correctProposalContribution) {
    Observations data({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    double eta=0.3;
    double chi=0.4;
    Parameters parameters({1, 1, 1});

    Hypergraph hypergraph(4);
    Parameters moveProbabilities {0.5, 0.3, 0.2};

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::EDGE;
    proposal.move = ADD;
    proposal.changedPairs = {{0, 1}};
    proposal.chosenTriplet = {0, 1, 0};

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
                log(edgeRemover.getReverseProbability({0, 1}, ADD))
                  - log(edgeAdder.getForwardProbability({0, 1}, ADD)),
                0.00001);
}


TEST_F(SixStepsHypergraph_testCase, when_removingEdge_expect_correctProposalContribution) {

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi);

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

TEST_F(SixStepsHypergraph_testCase, when_addingTriangle_expect_correctProposalContribution) {

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi);

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

TEST(SixStepsHypergraph, when_addingTriangleAndGraphHasNoTriangle_expect_correctProposalContribution) {
Observations data({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    double eta=0.3;
    double chi=0.4;
    Parameters parameters({1, 1, 1});
    Parameters moveProbabilities {0.5, 0.3, 0.2};

    Hypergraph hypergraph(4);
    hypergraph.addEdge(0, 1);

    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::TRIANGLE;
    proposal.move = ADD;
    proposal.changedPairs = {{0, 1}, {0, 2}, {1, 2}};
    proposal.chosenTriplet = {0, 1, 2};

    proposer.setProposal(proposal);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(triangleRemover.getReverseProbability({0, 1, 2}, ADD))
                - log(triangleAdder.getForwardProbability({0, 1, 2}, ADD)),
            0.00001);
}

TEST_F(SixStepsHypergraph_testCase, when_removingTriangle_expect_correctProposalContribution) {
    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi);

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

TEST_F(SixStepsHypergraph_testCase, when_cleaningAllTriangles_expect_correctProposalAndContribution) {
    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::HIDDEN_EDGES;
    proposal.move = REMOVE;
    proposal.changedPairs = {{0, 1}, {0, 2}, {3, 4}};

    proposer.setProposal({REMOVE, SixStepsHypergraphProposal::HIDDEN_EDGES, {}});
    proposer.proposeHiddenEdges();

    EXPECT_EQ(proposer.currentProposal.moveType, proposal.moveType);
    EXPECT_EQ(proposer.currentProposal.move, proposal.move);
    EXPECT_EQ(proposer.currentProposal.changedPairs, proposal.changedPairs);

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            3*log(chi)+5*log(1-chi)+log(eta)-log(1-eta),
            0.00001);
}

TEST_F(SixStepsHypergraph_testCase, when_dirtyingTriangles_expect_correctProposalAndContribution) {
    ObservationsWeightedEdgeChooser edgeAdder(data);
    UniformNonEdgeChooser edgeRemover(hypergraph);
    ObservationsPairwiseTriangleChooser triangleAdder(data);
    UniformTriangleChooser triangleRemover(hypergraph);

    HypergraphSixStepsProposer proposer(hypergraph, parameters, data, triangleAdder, triangleRemover, edgeAdder, edgeRemover, moveProbabilities, eta, chi);

    SixStepsHypergraphProposal proposal;
    proposal.moveType = SixStepsHypergraphProposal::HIDDEN_EDGES;
    proposal.move = ADD;
    proposal.unchangedPairsNumber = 0;
    proposer.setProposal(proposal);
    generator.seed(100);
    proposer.proposeHiddenEdges();

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
            log(1-eta)-log(eta)-2*log(chi)-3*log(1-chi),
            0.00001);
}
