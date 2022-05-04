#ifndef NITRIC_TWOSTEPS_TRIANGLE_H
#define NITRIC_TWOSTEPS_TRIANGLE_H


#include <stdexcept>
#include <random>

#include "GRIT/utility.h"
#include "GRIT/proposers/movetypes.h"
#include "proposer_base.h"
#include "GRIT/proposers/triangle-choosers/chooser_base.h"


namespace GRIT {

class TriangleTwoStepsProposer: public ProposerBase{
    Hypergraph& hypergraph;

    TriangleChooserBase& additionChooser;
    TriangleChooserBase& removalChooser;

    double eta;

    public:
        TwoStepsTriangleProposal currentProposal;

        TriangleTwoStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Observations& observations, TriangleChooserBase& additionChooser, TriangleChooserBase& removalChooser, double eta=0.5);
        TriangleTwoStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Parameters& hyperParameters, const Observations& observations, TriangleChooserBase& additionChooser, TriangleChooserBase& removalChooser, double eta=0.5);

        void generateProposal();
        double getLogAcceptanceContribution() const;
        bool applyStep();
        void recomputeProposersDistributions();

        void setProposal(Triplet triplet, AddRemoveMove move);
};

} //namespace GRIT

#endif
