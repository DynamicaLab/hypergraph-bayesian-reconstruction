#ifndef GRIT_PROPOSER_BASE_H
#define GRIT_PROPOSER_BASE_H


namespace GRIT {

class ProposerBase {
    public:
        virtual void generateProposal() = 0;
        virtual bool applyStep() = 0;
        virtual double getLogAcceptanceContribution() const = 0;
        virtual void recomputeProposersDistributions() = 0;

        virtual ~ProposerBase() {};
};

} // namespace GRIT

#endif
