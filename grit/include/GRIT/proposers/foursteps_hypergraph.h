#ifndef GRIT_HYPERGRAPH_FOURSTEPS_PROPOSER_H
#define GRIT_HYPERGRAPH_FOURSTEPS_PROPOSER_H


#include <stdexcept>
#include <random>

#include "GRIT/utility.h"
#include "GRIT/proposers/movetypes.h"
#include "proposer_base.h"
#include "GRIT/proposers/triangle-choosers/chooser_base.h"
#include "GRIT/proposers/edge-choosers/chooser_base.h"


namespace GRIT {

class HypergraphFourStepsProposer: public ProposerBase{
    Hypergraph& hypergraph;

    TriangleChooserBase& triangleAdder;
    TriangleChooserBase& triangleRemover;
    EdgeChooserBase& edgeAdder;
    EdgeChooserBase& edgeRemover;

    double eta;
    std::bernoulli_distribution movetypeDistribution;
    std::bernoulli_distribution hyperedgeTypeDistribution;

    public:
        FourStepsHypergraphProposal currentProposal;

        HypergraphFourStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Observations& observations,
                TriangleChooserBase& triangleAdder, TriangleChooserBase& triangleRemover,
                EdgeChooserBase& egdeAdder, EdgeChooserBase& edgeRemover,
                double eta=0.5, double xi=0.5);

        HypergraphFourStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Parameters& hyperParameters, const Observations& observations,
                TriangleChooserBase& triangleAdder, TriangleChooserBase& triangleRemover,
                EdgeChooserBase& edgeAdder, EdgeChooserBase& edgeRemover,
                double eta=0.5, double xi=0.5);

        void generateProposal();
        void proposeTriangle();
        void proposeEdge();

        double getLogAcceptanceContribution() const;
        bool applyStep();
        void recomputeProposersDistributions();

        void setProposal(const FourStepsHypergraphProposal& proposal) { currentProposal = proposal; };
};

} //namespace GRIT

#endif
