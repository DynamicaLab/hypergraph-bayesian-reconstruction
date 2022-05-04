#include "GRIT/proposers/twosteps_edges.h"


namespace GRIT {

EdgeTwoStepsProposer::EdgeTwoStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Observations& observations, EdgeChooserBase& additionChooser, EdgeChooserBase& removalChooser, double eta):
        hypergraph(hypergraph), eta(eta),
        currentProposal({ REMOVE, {0, 0} }),
        additionChooser(additionChooser), removalChooser(removalChooser)
{}

EdgeTwoStepsProposer::EdgeTwoStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Parameters& hyperParameters, const Observations& observations, EdgeChooserBase& additionChooser, EdgeChooserBase& removalChooser, double eta):
        hypergraph(hypergraph), eta(eta),
        currentProposal({ REMOVE, {0, 0} }),
        additionChooser(additionChooser), removalChooser(removalChooser)
{}


void EdgeTwoStepsProposer::generateProposal() {

    if (hypergraph.getEdgeNumber() == 0)
        currentProposal.move = ADD;
    else if (hypergraph.getEdgeNumber() == hypergraph.getMaximumEdgeNumber())
        currentProposal.move = REMOVE;
    else {
        bool addEdge = std::bernoulli_distribution(eta)(generator);
        currentProposal.move = AddRemoveMove(addEdge);
    }

    if (currentProposal.move == ADD)
        currentProposal.chosenEdge = additionChooser.choose();
    else
        currentProposal.chosenEdge = removalChooser.choose();
}

double EdgeTwoStepsProposer::getLogAcceptanceContribution() const {
    double logAcceptance = 0;
    if (currentProposal.move == ADD) {
        if (hypergraph.getEdgeNumber() == hypergraph.getMaximumEdgeNumber()-1)
            logAcceptance += -log(eta);
        else
            logAcceptance += log(1-eta) - log(eta);

        logAcceptance += log(removalChooser.getReverseProbability(currentProposal.chosenEdge, currentProposal.move));
        logAcceptance += -log(additionChooser.getForwardProbability(currentProposal.chosenEdge, currentProposal.move));
    }
    else if (currentProposal.move == REMOVE) {
        if (hypergraph.getEdgeNumber() == 1)
            logAcceptance += -log(1-eta);
        else
            logAcceptance += log(eta) - log(1-eta);

        logAcceptance += log(additionChooser.getReverseProbability(currentProposal.chosenEdge, currentProposal.move));
        logAcceptance += -log(removalChooser.getForwardProbability(currentProposal.chosenEdge, currentProposal.move));
    }
    return logAcceptance;
}

bool EdgeTwoStepsProposer::applyStep() {
    bool hypergraphChanged = false;

    const size_t& i = currentProposal.chosenEdge.first;
    const size_t& j = currentProposal.chosenEdge.second;

    if (i != j){
        additionChooser.updateProbabilities(currentProposal.chosenEdge, currentProposal.move);
        removalChooser.updateProbabilities(currentProposal.chosenEdge, currentProposal.move);
        if (currentProposal.move == ADD)
            hypergraphChanged = hypergraph.addEdge(i, j);
        else
            hypergraphChanged = hypergraph.removeEdge(i, j);
    }
    return hypergraphChanged;
}

void EdgeTwoStepsProposer::setProposal(Edge edge, AddRemoveMove move) {
    currentProposal.chosenEdge = edge;
    currentProposal.move = move;
}

void EdgeTwoStepsProposer::recomputeProposersDistributions() {
    additionChooser.recomputeDistribution();
    removalChooser.recomputeDistribution();
}

} //namespace GRIT
