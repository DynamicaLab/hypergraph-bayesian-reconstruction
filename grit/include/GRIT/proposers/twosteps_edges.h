#ifndef GRIT_TWOSTEPS_EDGE_H
#define GRIT_TWOSTEPS_EDGE_H


#include <random>
#include <stdexcept>

#include "GRIT/utility.h"
#include "GRIT/proposers/edge-choosers/chooser_base.h"
#include "proposer_base.h"
#include "GRIT/proposers/movetypes.h"


namespace GRIT {

class EdgeTwoStepsProposer: public ProposerBase{
    Hypergraph& hypergraph;

    EdgeChooserBase& additionChooser;
    EdgeChooserBase& removalChooser;

    double eta;

    public:
        TwoStepsEdgeProposal currentProposal;

        EdgeTwoStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Observations& observations, EdgeChooserBase& additionChooser, EdgeChooserBase& removalChooser, double eta=0.5);
        EdgeTwoStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Parameters& hyperParameters, const Observations& observations, EdgeChooserBase& additionChooser, EdgeChooserBase& removalChooser, double eta=0.5);

        void generateProposal();
        void setProposal(Edge edge, AddRemoveMove move);
        double getLogAcceptanceContribution() const;
        void recomputeProposersDistributions();
        bool applyStep();
};

} //namespace GRIT

#endif
