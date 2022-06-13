#ifndef GRIT_HYPERGRAPH_SIXSTEPS_PROPOSER_H
#define GRIT_HYPERGRAPH_SIXSTEPS_PROPOSER_H


#include <stdexcept>
#include <random>

#include "GRIT/utility.h"
#include "GRIT/proposers/movetypes.h"
#include "proposer_base.h"
#include "GRIT/proposers/triangle-choosers/chooser_base.h"
#include "GRIT/proposers/edge-choosers/chooser_base.h"


namespace GRIT {

class HypergraphSixStepsProposer: public ProposerBase{
    Hypergraph& hypergraph;

    TriangleChooserBase& triangleAdder;
    TriangleChooserBase& triangleRemover;
    EdgeChooserBase& edgeAdder;
    EdgeChooserBase& edgeRemover;

    double eta, chi_0, chi_1;
    size_t pairsUnder3edgeNumber=0;

    std::bernoulli_distribution addRemoveDistribution;
    std::discrete_distribution<int> moveTypeDistribution;

    public:
        SixStepsHypergraphProposal currentProposal;

        HypergraphSixStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Observations& observations,
                TriangleChooserBase& triangleAdder, TriangleChooserBase& triangleRemover,
                EdgeChooserBase& egdeAdder, EdgeChooserBase& edgeRemover,
                const std::vector<double>& moveProbabilities, double eta=0.5, double chi_0=0.99, double chi_1=0.01);

        HypergraphSixStepsProposer(Hypergraph& hypergraph, Parameters& parameters, const Parameters& hyperParameters, const Observations& observations,
                TriangleChooserBase& triangleAdder, TriangleChooserBase& triangleRemover,
                EdgeChooserBase& edgeAdder, EdgeChooserBase& edgeRemover,
                const std::vector<double>& moveProbabilities, double eta=0.5, double chi_0=0.99, double chi_1=0.01);

        void generateProposal();
        void proposeTriangle();
        void proposeEdge();
        void proposeHiddenEdges();

        double getLogAcceptanceContribution() const;
        bool applyStep();
        void recomputeProposersDistributions();

        void setProposal(const SixStepsHypergraphProposal& proposal) { currentProposal = proposal; };

    private:
        void updatePairHiddenEdgeMove(size_t i, size_t j, std::set<Edge>& unchangedPairs);

};

} //namespace GRIT

#endif
