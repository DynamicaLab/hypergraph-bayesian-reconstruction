#include <gtest/gtest.h>

#include "GRIT/utility.h"
#include "GRIT/proposers/twosteps_edges.h"
#include "GRIT/proposers/edge-choosers/observations_weighted_chooser.h"
#include "GRIT/proposers/edge-choosers/uniform_nonzero_chooser.h"


using namespace std;
using namespace GRIT;


TEST(TwoStepsEdgeProposal, when_addingEdge_expect_correctEtaContribution) {
    Observations data({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    double eta=0.3;
    Parameters parameters({1, 1, 1});

    Hypergraph graph(4);
    graph.addEdge(0, 1);
    graph.addMultiedge(0, 2, 2);
    graph.addEdge(1, 2);

    EdgeObservationsWeightedChooser adder(data);
    EdgeNonzeroUniformChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, data, adder, remover, eta);

    double forwardProbability, reverseProbability;
    proposer.setProposal({1, 3}, ADD);
    forwardProbability = log(adder.getForwardProbability({1, 3}, ADD));
    reverseProbability = log(remover.getReverseProbability({1, 3}, ADD));

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(), 
                reverseProbability-forwardProbability + log(1-eta) - log(eta),
                0.00001);
}

TEST(TwoStepsEdgeProposal, when_removingEdge_expect_correctEtaContribution) {
    Observations data({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    double eta=0.3;
    Parameters parameters({1, 1, 1});

    Hypergraph graph(4);
    graph.addEdge(0, 1);
    graph.addMultiedge(0, 2, 2);
    graph.addEdge(1, 2);

    EdgeObservationsWeightedChooser adder(data);
    EdgeNonzeroUniformChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, data, adder, remover, eta);

    double forwardProbability, reverseProbability;
    proposer.setProposal({1, 2}, REMOVE);
    forwardProbability = log(remover.getForwardProbability({1, 2}, REMOVE));
    reverseProbability = log(adder.getReverseProbability({1, 2}, REMOVE));

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(), 
                reverseProbability-forwardProbability + log(eta) - log(1-eta),
                0.00001);
}

TEST(TwoStepsEdgeProposal, when_noEdgeAndProposeToAddEdge_expect_noEtaContribution) {
    Observations data({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    double eta=0.3;
    Parameters parameters({1, 1, 1});

    Hypergraph graph(4);

    EdgeObservationsWeightedChooser adder(data);
    EdgeNonzeroUniformChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, data, adder, remover, eta);

    double forwardProbability, reverseProbability;
    proposer.setProposal({1, 2}, ADD);
    forwardProbability = log(adder.getForwardProbability({1, 2}, ADD));
    reverseProbability = log(remover.getReverseProbability({1, 2}, ADD));

    EXPECT_NEAR(proposer.getLogAcceptanceContribution(), 
                reverseProbability-forwardProbability,
                0.00001);
}

TEST(TwoStepsEdgeProposal, when_addingEdgeAndApplyingProposal_expect_multiplicityIncrement){
    Observations data({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});
    Hypergraph graph(4);
    graph.addEdge(0, 1);
    graph.addMultiedge(0, 2, 2);
    graph.addEdge(1, 2);

    EdgeObservationsWeightedChooser adder(data);
    EdgeNonzeroUniformChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, data, adder, remover);

    proposer.setProposal(Edge(1, 3), ADD);

    //auto previousAdjacency (graph.getAdjacencyMatrix());
    //proposer.applyStep();
    //auto newAdjacency (graph.getAdjacencyMatrix());

    //for (size_t i=0; i<4; i++) {
        //for (size_t j=0; j<4; j++) {
            //if (i==1 && j==3 || i==3 && j==1)
                //EXPECT_EQ(previousAdjacency[i][j]+1, newAdjacency[i][j]);
            //else
                //EXPECT_EQ(previousAdjacency[i][j], newAdjacency[i][j]);
        //}
    //}
}

TEST(TwoStepsEdgeProposal, when_removingEdgeAndApplyingProposal_expect_multiplicityDecrement){
    Observations data({{0, 1, 2, 0},
               {1, 0, 0, 1},
               {2, 0, 0, 1},
               {0, 1, 1, 0} });
    Parameters parameters({1, 1, 1});
    Hypergraph graph(4);
    graph.addEdge(0, 1);
    graph.addMultiedge(0, 2, 2);
    graph.addEdge(1, 2);

    EdgeObservationsWeightedChooser adder(data);
    EdgeNonzeroUniformChooser remover(graph);
    EdgeTwoStepsProposer proposer(graph, parameters, data, adder, remover);

    proposer.setProposal(Edge(1, 2), REMOVE);

    //auto previousAdjacency (graph.getAdjacencyMatrix());
    //proposer.applyStep();
    //auto newAdjacency (graph.getAdjacencyMatrix());

    //for (size_t i=0; i<4; i++) {
        //for (size_t j=0; j<4; j++) {
            //if (i==1 && j==2 || i==2 && j==1)
                //EXPECT_EQ(previousAdjacency[i][j]-1, newAdjacency[i][j]);
            //else
                //EXPECT_EQ(previousAdjacency[i][j], newAdjacency[i][j]);
        //}
    //}
}
