#ifndef GRIT_POISSON_GRAPH_TRUNCATED_GAMMA_PRIOR_H
#define GRIT_POISSON_GRAPH_TRUNCATED_GAMMA_PRIOR_H

#include <stdexcept>
#include "GRIT/utility.h"


namespace GRIT {

class PoissonGraph_BetaAndGammaPriors {
    const size_t qIndex = 0;
    const size_t mu0Index = 2;
    const Parameters& parameters;
    const Parameters& hyperParameters;

    public:
        PoissonGraph_BetaAndGammaPriors(const Parameters& parameters, const Parameters& hyperParameters):
                parameters(parameters), hyperParameters(hyperParameters) {
                    if (hyperParameters.size() != 10)
                        throw std::logic_error("Graph prior: incorrect number of parameters. 10 required but " + std::to_string(hyperParameters.size()) + " given.");
                };

        double operator()() const {
            const double& q = parameters[qIndex];
            const double mu[2] = {parameters[mu0Index], parameters[mu0Index+1]};

            double logLikelihood = 0;
            logLikelihood += (hyperParameters[0]-1)*log(q) + (hyperParameters[1]-1) * log(1-q);
            logLikelihood += (hyperParameters[4]-1)*log(mu[0]) - hyperParameters[5] * mu[0];
            logLikelihood += truncGammaLogProb(mu[1], mu[0], hyperParameters[6], hyperParameters[7]);
            return logLikelihood;
        }
};

} // namespace GRIT

#endif
