#ifndef GRIT_POISSON_HYPERGRAPH_TRUNCATED_GAMMA_PRIOR_H
#define GRIT_POISSON_HYPERGRAPH_TRUNCATED_GAMMA_PRIOR_H

#include <stdexcept>
#include "GRIT/utility.h"


namespace GRIT {

class PoissonHypergraph_BetaAndGammaPriors {
    const size_t pIndex = 0;
    const size_t qIndex = 0;
    const size_t mu0Index = 2;
    const Parameters& parameters;
    const Parameters& hyperParameters;

    public:
        PoissonHypergraph_BetaAndGammaPriors(const Parameters& parameters, const Parameters& hyperParameters):
                parameters(parameters), hyperParameters(hyperParameters) {
                    if (hyperParameters.size() != 10)
                        throw std::logic_error("Hypergraph prior: incorrect number of parameters. 10 required but " + std::to_string(hyperParameters.size()) + " given.");
                };

        double operator()() const {
            const double& p = parameters[pIndex];
            const double& q = parameters[qIndex];
            const double mu[3] = {parameters[mu0Index], parameters[mu0Index+1], parameters[mu0Index+2]};

            double logLikelihood = 0;
            logLikelihood += (hyperParameters[0]-1)*log(p) + (hyperParameters[1]-1) * log(1-p);
            logLikelihood += (hyperParameters[2]-1)*log(q) + (hyperParameters[3]-1) * log(1-q);
            logLikelihood += (hyperParameters[4]-1)*log(mu[0]) - hyperParameters[5] * mu[0];
            logLikelihood += truncGammaLogProb(mu[1], mu[0], hyperParameters[6], hyperParameters[7]);
            logLikelihood += truncGammaLogProb(mu[2], mu[0], hyperParameters[8], hyperParameters[9]);
            return logLikelihood;
        }
};

} // namespace GRIT

#endif
