#include <gtest/gtest.h>

#include "GRIT/utility.h"
#include "GRIT/proposers/twosteps_edges.h"
#include "GRIT/proposers/edge-choosers/weighted_unique_chooser.h"
#include "GRIT/proposers/edge-choosers/uniform_edge_chooser.h"


using namespace std;
using namespace GRIT;


static const double eta=0.3;


TEST(TwoStepsEdgeProposal, when_addingEdge_expect_correctEtaContribution) {
    Observations observations({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});

    Hypergraph graph(4);
    graph.addEdge(0, 1);
    graph.addMultiedge(0, 2, 2);
    graph.addEdge(1, 2);

    ObservationsWeightedUniqueEdgeChooser adder(observations, graph);
    UniformNonEdgeChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, observations, adder, remover, eta);

    double forwardProbability, reverseProbability;
    proposer.setProposal({1, 3}, ADD);
    forwardProbability = log(adder.getForwardProbability({1, 3}, ADD));
    reverseProbability = log(remover.getReverseProbability({1, 3}, ADD));

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
                reverseProbability-forwardProbability + log(1-eta) - log(eta),
                0.00001);
}

TEST(TwoStepsEdgeProposal, when_proposingCompleteGraph_expect_onewayEtaContribution) {
    Observations observations({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});

    Hypergraph graph(4);
    for (size_t i=0; i<4; i++)
        for (size_t j=i+1; j<4; j++)
            if (!(i==1 && j==2))
                graph.addEdge(i, j);

    ObservationsWeightedUniqueEdgeChooser adder(observations, graph);
    UniformNonEdgeChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, observations, adder, remover, eta);

    double forwardProbability, reverseProbability;
    proposer.setProposal({1, 2}, ADD);
    forwardProbability = log(adder.getForwardProbability({1, 2}, ADD));
    reverseProbability = log(remover.getReverseProbability({1, 2}, ADD));

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
                reverseProbability-forwardProbability - log(eta),
                0.00001);
}

TEST(TwoStepsEdgeProposal, when_removingEdge_expect_correctEtaContribution) {
    Observations observations({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});

    Hypergraph graph(4);
    graph.addEdge(0, 1);
    graph.addMultiedge(0, 2, 2);
    graph.addEdge(1, 2);

    ObservationsWeightedUniqueEdgeChooser adder(observations, graph);
    UniformNonEdgeChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, observations, adder, remover, eta);

    double forwardProbability, reverseProbability;
    proposer.setProposal({1, 2}, REMOVE);
    forwardProbability = log(remover.getForwardProbability({1, 2}, REMOVE));
    reverseProbability = log(adder.getReverseProbability({1, 2}, REMOVE));

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
                reverseProbability-forwardProbability + log(eta) - log(1-eta),
                0.00001);
}

TEST(TwoStepsEdgeProposal, when_proposingEmptyGraph_expect_onewayEtaContribution) {
    Observations observations({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});

    Hypergraph graph(4);
    graph.addEdge(1, 2);

    ObservationsWeightedUniqueEdgeChooser adder(observations, graph);
    UniformNonEdgeChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, observations, adder, remover, eta);

    double forwardProbability, reverseProbability;
    proposer.setProposal({1, 2}, REMOVE);
    forwardProbability = log(remover.getForwardProbability({1, 2}, REMOVE));
    reverseProbability = log(adder.getReverseProbability({1, 2}, REMOVE));

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(),
                reverseProbability-forwardProbability - log(1-eta),
                0.00001);
}

TEST(TwoStepsEdgeProposal, when_addingEdgeAndApplyingProposal_expect_multiplicityIncrement){
    Observations observations({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});
    Hypergraph graph(4);
    graph.addEdge(0, 1);
    graph.addMultiedge(0, 2, 2);
    graph.addEdge(1, 2);

    ObservationsWeightedUniqueEdgeChooser adder(observations, graph);
    UniformNonEdgeChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, observations, adder, remover);

    proposer.setProposal(Edge(1, 3), ADD);

    auto previousGraph(graph);
    proposer.applyStep();

    for (size_t i=0; i<4; i++) {
        for (size_t j=0; j<4; j++) {
            if (i==1 && j==3 || i==3 && j==1)
                EXPECT_EQ(previousGraph.getEdgeMultiplicity(i, j)+1, graph.getEdgeMultiplicity(i, j));
            else
                EXPECT_EQ(previousGraph.getEdgeMultiplicity(i, j), graph.getEdgeMultiplicity(i, j));
        }
    }
}

TEST(TwoStepsEdgeProposal, when_removingEdgeAndApplyingProposal_expect_multiplicityDecrement){
    Observations observations({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});
    Hypergraph graph(4);
    graph.addEdge(0, 1);
    graph.addMultiedge(0, 2, 2);
    graph.addEdge(1, 2);

    ObservationsWeightedUniqueEdgeChooser adder(observations, graph);
    UniformNonEdgeChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, observations, adder, remover);

    proposer.setProposal(Edge(1, 2), REMOVE);

    auto previousGraph(graph);
    proposer.applyStep();

    for (size_t i=0; i<4; i++) {
        for (size_t j=0; j<4; j++) {
            if (i==1 && j==2 || i==2 && j==1)
                EXPECT_EQ(previousGraph.getEdgeMultiplicity(i, j)-1, graph.getEdgeMultiplicity(i, j));
            else
                EXPECT_EQ(previousGraph.getEdgeMultiplicity(i, j), graph.getEdgeMultiplicity(i, j));
        }
    }
}
