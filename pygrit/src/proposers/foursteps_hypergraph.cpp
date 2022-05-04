#include "GRIT/proposers/foursteps_hypergraph.h"


namespace GRIT {

HypergraphFourStepsProposer::HypergraphFourStepsProposer(
                Hypergraph& hypergraph, Parameters& parameters, const Observations& observations,
                TriangleChooserBase& triangleAdder, TriangleChooserBase& triangleRemover,
                EdgeChooserBase& edgeAdder, EdgeChooserBase& edgeRemover,
                double eta, double xi):
        hypergraph(hypergraph), eta(eta),
        triangleAdder(triangleAdder), triangleRemover(triangleRemover),
        edgeAdder(edgeAdder), edgeRemover(edgeRemover)
{
    movetypeDistribution = std::bernoulli_distribution(eta);
    hyperedgeTypeDistribution = std::bernoulli_distribution(xi);
}

HypergraphFourStepsProposer::HypergraphFourStepsProposer(
                Hypergraph& hypergraph, Parameters& parameters, const Parameters& hyperParameters, const Observations& observations,
                TriangleChooserBase& triangleAdder, TriangleChooserBase& triangleRemover,
                EdgeChooserBase& edgeAdder, EdgeChooserBase& edgeRemover,
                double eta, double xi):
        hypergraph(hypergraph), eta(eta),
        triangleAdder(triangleAdder), triangleRemover(triangleRemover),
        edgeAdder(edgeAdder), edgeRemover(edgeRemover)
{
    movetypeDistribution = std::bernoulli_distribution(eta);
    hyperedgeTypeDistribution = std::bernoulli_distribution(xi);
}


void HypergraphFourStepsProposer::generateProposal() {
    bool edgetype = hyperedgeTypeDistribution(generator);
    currentProposal.move = AddRemoveMove(movetypeDistribution(generator));

    if (edgetype == 1) {
        currentProposal.moveType = FourStepsHypergraphProposal::TRIANGLE;
        proposeTriangle();
    }
    else {
        currentProposal.moveType = FourStepsHypergraphProposal::EDGE;
        proposeEdge();
    }
}

void HypergraphFourStepsProposer::proposeTriangle() {
    if (hypergraph.getTriangleNumber() == 0)
        currentProposal.move = ADD;


    Triplet chosenTriplet;
    if (currentProposal.move == ADD)
        chosenTriplet = triangleAdder.choose();
    else
        chosenTriplet = triangleRemover.choose();

    currentProposal.i = chosenTriplet.i;
    currentProposal.j = chosenTriplet.j;
    currentProposal.k = chosenTriplet.k;
}

void HypergraphFourStepsProposer::proposeEdge() {
    if (hypergraph.getEdgeNumber() == 0)
        currentProposal.move = ADD;


    Edge chosenEdge {0, 0};
    if (currentProposal.move == ADD)
        chosenEdge = edgeAdder.choose();
    else
        chosenEdge = edgeRemover.choose();

    currentProposal.i = chosenEdge.first;
    currentProposal.j = chosenEdge.second;
    currentProposal.k = -1;
}

double HypergraphFourStepsProposer::getLogAcceptanceContribution() const {
    if (currentProposal.k == -1 && currentProposal.moveType == FourStepsHypergraphProposal::TRIANGLE)
        throw std::logic_error("UNEXPECTED ERROR: k is -1 for a triangle");
    size_t i = currentProposal.i;
    size_t j = currentProposal.j;
    size_t k = currentProposal.k;

    double logAcceptance = 0;


    if (currentProposal.move == ADD) {
        if (currentProposal.moveType == FourStepsHypergraphProposal::EDGE) {
            if (hypergraph.getEdgeNumber() > 0)
                logAcceptance += log(1-eta) - log(eta);

            logAcceptance += log(edgeRemover.getReverseProbability({i, j}, ADD));
            logAcceptance += -log(edgeAdder.getForwardProbability({i, j}, ADD));
        }
        else if (currentProposal.moveType == FourStepsHypergraphProposal::TRIANGLE) {
            if (hypergraph.getTriangleNumber() > 0)
                logAcceptance += log(1-eta) - log(eta);

            logAcceptance += log(triangleRemover.getReverseProbability({i, j, k}, ADD));
            logAcceptance += -log(triangleAdder.getForwardProbability({i, j, k}, ADD));
        }
    }
    else {
        logAcceptance += log(eta) - log(1-eta);

        if (currentProposal.moveType == FourStepsHypergraphProposal::EDGE) {
            logAcceptance += log(edgeAdder.getReverseProbability({i, j}, REMOVE));
            logAcceptance += -log(edgeRemover.getForwardProbability({i, j}, REMOVE));
        }
        else if (currentProposal.moveType == FourStepsHypergraphProposal::TRIANGLE) {
            logAcceptance += log(triangleAdder.getReverseProbability({i, j, k}, REMOVE));
            logAcceptance += -log(triangleRemover.getForwardProbability({i, j, k}, REMOVE));
        }
    }
    return logAcceptance;
}

bool HypergraphFourStepsProposer::applyStep(){
    bool hypergraphChanged = false;

    const auto& i = currentProposal.i;
    const auto& j = currentProposal.j;
    const auto& k = currentProposal.k;


    if ( i!=j && currentProposal.moveType == FourStepsHypergraphProposal::EDGE){
        edgeAdder.updateProbabilities({i, j}, currentProposal.move);
        edgeRemover.updateProbabilities({i, j}, currentProposal.move);
        if (currentProposal.move == ADD)
            hypergraphChanged = hypergraph.addEdge(i, j);
        else
            hypergraphChanged = hypergraph.removeEdge(i, j);
    }

    if ( !(i==j || i==k || j==k) && currentProposal.moveType == FourStepsHypergraphProposal::TRIANGLE){
        triangleAdder.updateProbabilities({(size_t) i, (size_t) j, (size_t) k}, currentProposal.move);
        triangleRemover.updateProbabilities({(size_t) i, (size_t) j, (size_t) k}, currentProposal.move);
        if (currentProposal.move == ADD)
            hypergraphChanged = hypergraph.addTriangle({(size_t) i, (size_t) j, (size_t) k});
        else
            hypergraphChanged = hypergraph.removeTriangle({(size_t) i, (size_t) j, (size_t) k});
    }
    return hypergraphChanged;
}

void HypergraphFourStepsProposer::recomputeProposersDistributions() {
    edgeAdder.recomputeDistribution();
    edgeRemover.recomputeDistribution();
    triangleAdder.recomputeDistribution();
    triangleRemover.recomputeDistribution();
}

} //namespace GRIT
