#include <algorithm>
#include "GRIT/proposers/sixsteps_hypergraph.h"


namespace GRIT {

HypergraphSixStepsProposer::HypergraphSixStepsProposer(
                Hypergraph& hypergraph, Parameters& parameters, const Observations& observations,
                TriangleChooserBase& triangleAdder, TriangleChooserBase& triangleRemover,
                EdgeChooserBase& edgeAdder, EdgeChooserBase& edgeRemover,
                const std::vector<double>& moveProbabilities, double eta, double chi_0, double chi_1):
        hypergraph(hypergraph), eta(eta), chi_0(chi_0), chi_1(chi_1),
        triangleAdder(triangleAdder), triangleRemover(triangleRemover),
        edgeAdder(edgeAdder), edgeRemover(edgeRemover)
{
    addRemoveDistribution = std::bernoulli_distribution(eta);

    if (moveProbabilities.size() != 3)
        throw std::logic_error("HypergraphSixStepsProposer: Incorrect number of move probabilities. There were " + std::to_string(moveProbabilities.size())
                                + " given and 3 are required.");
    moveTypeDistribution = std::discrete_distribution<int> {moveProbabilities.begin(), moveProbabilities.end()};
}

HypergraphSixStepsProposer::HypergraphSixStepsProposer(
                Hypergraph& hypergraph, Parameters& parameters, const Parameters& hyperParameters, const Observations& observations,
                TriangleChooserBase& triangleAdder, TriangleChooserBase& triangleRemover,
                EdgeChooserBase& edgeAdder, EdgeChooserBase& edgeRemover,
                const std::vector<double>& moveProbabilities, double eta, double chi_0, double chi_1):
        hypergraph(hypergraph), eta(eta), chi_0(chi_0), chi_1(chi_1),
        triangleAdder(triangleAdder), triangleRemover(triangleRemover),
        edgeAdder(edgeAdder), edgeRemover(edgeRemover)
{
    addRemoveDistribution = std::bernoulli_distribution(eta);

    if (moveProbabilities.size() != 3)
        throw std::logic_error("HypergraphSixStepsProposer: Incorrect number of move probabilities. There were " + std::to_string(moveProbabilities.size())
                                + " given and 3 are required.");
    moveTypeDistribution = std::discrete_distribution<int> {moveProbabilities.begin(), moveProbabilities.end()};
}


void HypergraphSixStepsProposer::generateProposal() {
    currentProposal.changedPairs.clear();
    currentProposal.unchangedPairsNumber = 0;

    int moveType = moveTypeDistribution(generator);
    currentProposal.move = AddRemoveMove(addRemoveDistribution(generator));

    if (moveType == 0) {
        currentProposal.moveType = SixStepsHypergraphProposal::TRIANGLE;
        proposeTriangle();
    }
    else if (moveType == 1) {
        currentProposal.moveType = SixStepsHypergraphProposal::EDGE;
        proposeEdge();
    }
    else if (moveType == 2) {
        currentProposal.moveType = SixStepsHypergraphProposal::HIDDEN_EDGES;
        proposeHiddenEdges();
    }
    else
        throw std::logic_error("Move of type " + std::to_string(moveType) + " doesn't exist.");
}

void HypergraphSixStepsProposer::proposeTriangle() {
    if (hypergraph.getTriangleNumber() == 0)
        currentProposal.move = ADD;
    else if (hypergraph.getTriangleNumber() == hypergraph.getMaximumTriangleNumber())
        currentProposal.move = REMOVE;

    if (currentProposal.move == ADD)
        currentProposal.chosenTriplet = triangleAdder.choose();
    else
        currentProposal.chosenTriplet = triangleRemover.choose();

    auto& i=currentProposal.chosenTriplet.i, j=currentProposal.chosenTriplet.j, k=currentProposal.chosenTriplet.k;
    currentProposal.changedPairs = { {i, j}, {i, k}, {j, k} };
}

void HypergraphSixStepsProposer::proposeEdge() {
    if (hypergraph.getEdgeNumber() == 0)
        currentProposal.move = ADD;
    else if (hypergraph.getEdgeNumber() == hypergraph.getMaximumEdgeNumber())
        currentProposal.move = REMOVE;

    Edge chosenEdge {0, 0};
    if (currentProposal.move == ADD)
        chosenEdge = edgeAdder.choose();
    else
        chosenEdge = edgeRemover.choose();

    currentProposal.chosenTriplet.i = chosenEdge.first;
    currentProposal.chosenTriplet.j = chosenEdge.second;
    currentProposal.changedPairs = { chosenEdge };
}

void HypergraphSixStepsProposer::proposeHiddenEdges() {
    std::set<Edge> unchangedPairs;

    for (size_t i=0; i<hypergraph.getSize(); i++) {
        for (auto triangleNeighbour: hypergraph.getTrianglesFrom(i)) {
            const auto& j = triangleNeighbour.first;
            if (i<j) {
                updatePairHiddenEdgeMove(i, j, unchangedPairs);
                for (auto k: triangleNeighbour.second) {
                    updatePairHiddenEdgeMove(i, k, unchangedPairs);
                    updatePairHiddenEdgeMove(j, k, unchangedPairs);
                }
            }
        }
    }
    currentProposal.unchangedPairsNumber = unchangedPairs.size();
    currentProposal.maximumChangedPairsNumber = currentProposal.changedPairs.size();

    if (currentProposal.changedPairs.size() < 2)
        generateProposal();

    else {
        double chi = currentProposal.move == REMOVE ? chi_0: chi_1;
        const size_t changedPairsNumber = drawFromShiftedGeometricDistribution(chi, currentProposal.changedPairs.size());

        std::list<Edge> chosenPairs;
        std::sample(currentProposal.changedPairs.begin(), currentProposal.changedPairs.end(), std::back_insert_iterator<std::list<Edge>>(chosenPairs), changedPairsNumber, generator);

        currentProposal.unchangedPairsNumber += currentProposal.changedPairs.size() - chosenPairs.size();
        currentProposal.changedPairs = std::set<Edge>(chosenPairs.begin(), chosenPairs.end());
    }
}

void HypergraphSixStepsProposer::updatePairHiddenEdgeMove(size_t i, size_t j, std::set<Edge>& unchangedPairs) {
    if (currentProposal.move == ADD && !hypergraph.isEdge(i, j) ||
            currentProposal.move == REMOVE && hypergraph.isEdge(i, j))
        currentProposal.changedPairs.insert({i, j});
    else
        unchangedPairs.insert({i, j});
}

double HypergraphSixStepsProposer::getLogAcceptanceContribution() const {
    const size_t& i = currentProposal.chosenTriplet.i;
    const size_t& j = currentProposal.chosenTriplet.j;
    const size_t& k = currentProposal.chosenTriplet.k;

    const size_t changedPairsNumber = currentProposal.changedPairs.size();
    const size_t& unchangedPairsNumber = currentProposal.unchangedPairsNumber;

    const size_t& edgeNumber(hypergraph.getEdgeNumber()), triangleNumber(hypergraph.getTriangleNumber());
    size_t maximumEdgeNumber(hypergraph.getMaximumEdgeNumber()), maximumTriangleNumber(hypergraph.getMaximumTriangleNumber());

    double logAcceptance = 0;


    if (currentProposal.move == ADD) {
        if (currentProposal.moveType == SixStepsHypergraphProposal::EDGE) {
            if (edgeNumber == maximumEdgeNumber-1)
                logAcceptance += -log(eta);
            else
                logAcceptance += log(1-eta) - log(eta);

            logAcceptance += log(edgeRemover.getReverseProbability({i, j}, ADD));
            logAcceptance += -log(edgeAdder.getForwardProbability({i, j}, ADD));
        }
        else if (currentProposal.moveType == SixStepsHypergraphProposal::TRIANGLE) {
            if (triangleNumber == maximumTriangleNumber-1)
                logAcceptance += -log(eta);
            else
                logAcceptance += log(1-eta) - log(eta);

            logAcceptance += log(triangleRemover.getReverseProbability({i, j, k}, ADD));
            logAcceptance += -log(triangleAdder.getForwardProbability({i, j, k}, ADD));
        }
        else if (currentProposal.moveType == SixStepsHypergraphProposal::HIDDEN_EDGES) {
            logAcceptance += log(1-eta) - log(eta);
                            + (changedPairsNumber-2) * (log(chi_0)-log(chi_1)) + log(1-chi_0) - log(1-chi_1)
                            + log(1-pow(chi_1, currentProposal.maximumChangedPairsNumber-1)) - log(1-pow(chi_0, unchangedPairsNumber-(currentProposal.maximumChangedPairsNumber-changedPairsNumber) + changedPairsNumber-1));
        }
    }
    else if (currentProposal.move == REMOVE) {
        if (currentProposal.moveType == SixStepsHypergraphProposal::EDGE) {
            if (edgeNumber == 1)
                logAcceptance += -log(1-eta);
            else
                logAcceptance += log(eta) - log(1-eta);

            logAcceptance += log(edgeAdder.getReverseProbability({i, j}, REMOVE));
            logAcceptance += -log(edgeRemover.getForwardProbability({i, j}, REMOVE));
        }
        else if (currentProposal.moveType == SixStepsHypergraphProposal::TRIANGLE) {
            if (triangleNumber == 1)
                logAcceptance += -log(1-eta);
            else
                logAcceptance += log(eta) - log(1-eta);

            logAcceptance += log(triangleAdder.getReverseProbability({i, j, k}, REMOVE));
            logAcceptance += -log(triangleRemover.getForwardProbability({i, j, k}, REMOVE));
        }
        else if (currentProposal.moveType == SixStepsHypergraphProposal::HIDDEN_EDGES) {
            logAcceptance += log(eta) - log(1-eta);
                            + (changedPairsNumber-2) * (log(chi_1)-log(chi_0)) + log(1-chi_1) - log(1-chi_0)
                            + log(1-pow(chi_0, currentProposal.maximumChangedPairsNumber-1)) - log(1-pow(chi_1, unchangedPairsNumber-(currentProposal.maximumChangedPairsNumber-changedPairsNumber) + changedPairsNumber-1));
        }
    }

    return logAcceptance;
}

bool HypergraphSixStepsProposer::applyStep(){
    bool hypergraphChanged = false;

    const size_t& i = currentProposal.chosenTriplet.i;
    const size_t& j = currentProposal.chosenTriplet.j;
    const size_t& k = currentProposal.chosenTriplet.k;


    if ( i!=j && currentProposal.moveType == SixStepsHypergraphProposal::EDGE) {
        edgeAdder.updateProbabilities({i, j}, currentProposal.move);
        edgeRemover.updateProbabilities({i, j}, currentProposal.move);
        if (currentProposal.move == ADD)
            hypergraphChanged = hypergraph.addEdge(i, j);
        else
            hypergraphChanged = hypergraph.removeEdge(i, j);
    }
    else if (currentProposal.moveType == SixStepsHypergraphProposal::TRIANGLE) {
        if ( !(i==j || i==k || j==k) ) {
            triangleAdder.  updateProbabilities({i, j, k}, currentProposal.move);
            triangleRemover.updateProbabilities({i, j, k}, currentProposal.move);

            if (currentProposal.move == ADD)
                hypergraphChanged = hypergraph.addTriangle({i, j, k});
            else
                hypergraphChanged = hypergraph.removeTriangle({i, j, k});
        }
    }
    else if (currentProposal.moveType == SixStepsHypergraphProposal::HIDDEN_EDGES) {
        hypergraphChanged = true;

        for (auto edge: currentProposal.changedPairs) {
            edgeAdder.updateProbabilities(edge, currentProposal.move);
            edgeRemover.updateProbabilities(edge, currentProposal.move);

            if (currentProposal.move == ADD) {
                if (!hypergraph.addEdge(edge.first, edge.second))
                    throw std::logic_error("HypergraphSixStepsProposer: Dirty hyperedges move error."
                            " An existent edge is part of changed edges.");
            }
            else if (currentProposal.move == REMOVE)
                if (!hypergraph.removeEdge(edge.first, edge.second))
                    throw std::logic_error("HypergraphSixStepsProposer: Clean hyperedges move error."
                            " An inexistent edge is part of changed edges.");
        }
    }
    return hypergraphChanged;
}

void HypergraphSixStepsProposer::recomputeProposersDistributions() {
    edgeAdder.recomputeDistribution();
    edgeRemover.recomputeDistribution();
    triangleAdder.recomputeDistribution();
    triangleRemover.recomputeDistribution();
}

} //namespace GRIT
