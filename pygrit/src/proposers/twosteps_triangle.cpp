#include "GRIT/proposers/twosteps_triangle.h"


namespace GRIT {

TriangleTwoStepsProposer::TriangleTwoStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Observations& observations, TriangleChooserBase& additionChooser, TriangleChooserBase& removalChooser, double eta):
        hypergraph(hypergraph),
        additionChooser(additionChooser), removalChooser(removalChooser),
        eta(eta)
{}

TriangleTwoStepsProposer::TriangleTwoStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Parameters& hyperParameters, const Observations& observations, TriangleChooserBase& additionChooser, TriangleChooserBase& removalChooser, double eta):
        hypergraph(hypergraph),
        additionChooser(additionChooser), removalChooser(removalChooser),
        eta(eta)
{}


void TriangleTwoStepsProposer::generateProposal() {
    bool addTriangle = std::bernoulli_distribution(eta)(generator);

    currentProposal.move = AddRemoveMove(addTriangle);

    if (hypergraph.getTriangleNumber() == 0) currentProposal.move = ADD;

    if (currentProposal.move == ADD)
        currentProposal.chosenTriplet = additionChooser.choose();
    else
        currentProposal.chosenTriplet = removalChooser.choose();
}

double TriangleTwoStepsProposer::getLogAcceptanceContribution() const {
    long double logAcceptance = 0;

    if (currentProposal.move == ADD) {
        if (hypergraph.getTriangleNumber() > 0)
            logAcceptance += log(1-eta) - log(eta);
        logAcceptance += log(removalChooser.getReverseProbability(currentProposal.chosenTriplet, currentProposal.move));
        logAcceptance += -log(additionChooser.getForwardProbability(currentProposal.chosenTriplet, currentProposal.move));
    }
    else {
        logAcceptance += log(eta) - log(1-eta);
        logAcceptance += log(additionChooser.getReverseProbability(currentProposal.chosenTriplet, currentProposal.move));
        logAcceptance += -log(removalChooser.getForwardProbability(currentProposal.chosenTriplet, currentProposal.move));
    }
    return logAcceptance;
}

bool TriangleTwoStepsProposer::applyStep(){
    bool hypergraphChanged = false;
    const size_t& i = currentProposal.chosenTriplet.i;
    const size_t& j = currentProposal.chosenTriplet.j;
    const size_t& k = currentProposal.chosenTriplet.k;

    if ( !(i==j || i==k || j==k) ){
        additionChooser.updateProbabilities(currentProposal.chosenTriplet, currentProposal.move);
        removalChooser.updateProbabilities(currentProposal.chosenTriplet, currentProposal.move);
        if (currentProposal.move == ADD)
            hypergraphChanged = hypergraph.addTriangle(currentProposal.chosenTriplet);
        else
            hypergraphChanged = hypergraph.removeTriangle(currentProposal.chosenTriplet);
    }
    return hypergraphChanged;
}

void TriangleTwoStepsProposer::setProposal(Triplet triplet, AddRemoveMove move) {
    currentProposal.chosenTriplet = triplet;
    currentProposal.move = move;
}

void TriangleTwoStepsProposer::recomputeProposersDistributions() {
    additionChooser.recomputeDistribution();
    removalChooser.recomputeDistribution();
}

} //namespace GRIT
