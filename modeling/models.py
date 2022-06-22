from abc import ABC, abstractmethod
import warnings
import os
import numpy as np
from numpy.lib.twodim_base import triu_indices
from scipy.special import loggamma
from scipy import stats, optimize

from .output import remove_all_chains_but, chain_directory_prefix, erase_sample
import pygrit


def get_occurences_in_observations(observations):
    counts = np.bincount(observations.astype(np.int64).ravel())
    mask = counts != 0
    n = counts.shape[0]

    return np.arange(n)[mask], counts[mask]


# Adapted https://stackoverflow.com/questions/11130156/suppress-stdout-stderr-print-from-python-functions
class mute_output(object):
    '''
    A context manager for doing a "deep suppression" of stdout and stderr in
    Python, i.e. will suppress all print, even if the print originates in a
    compiled C/Fortran sub-function.
       This will not suppress raised exceptions, since exceptions are printed
    to stderr just before a script exits, and after the context manager has
    exited (at least, I think that is why it lets exceptions through).

    '''

    def __init__(self, stdout=True, stderr=False):
        self.suppres_stdout = stdout
        self.suppres_stderr = stderr

        # Open a pair of null files
        self.null_fds = [os.open(os.devnull, os.O_RDWR) for x in range(2)]
        # Save the actual stdout (1) and stderr (2) file descriptors.
        self.save_fds = (os.dup(1), os.dup(2))

    def __enter__(self):
        # Assign the null pointers to stdout and stderr.
        if self.suppres_stdout:
            os.dup2(self.null_fds[0], 1)
        if self.suppres_stderr:
            os.dup2(self.null_fds[1], 2)

    def __exit__(self, *_):
        # Re-assign the real stdout/stderr back to (1) and (2)
        if self.suppres_stdout:
            os.dup2(self.save_fds[0], 1)
        if self.suppres_stderr:
            os.dup2(self.save_fds[1], 2)
        # Close the null files
        os.close(self.null_fds[0])
        os.close(self.null_fds[1])


class InferenceModel(ABC):
    def __init__(self, config):
        self.config = config
        self.n = self.config["vertex number"]
        self.sampler = None

    def generate_observations(self, hypergraph, parameters):
        mu0, mu1, mu2 = parameters[2:]
        return pygrit.generate_poisson_observations(hypergraph, mu0, mu1, mu2, self.with_correlation)

    @abstractmethod
    def get_edgetype_probabilities(self, parameters):
        pass

    @abstractmethod
    def get_parameter_names(self):
        pass

    @abstractmethod
    def adjust_hypergraph(self, hypergraph):
        pass

    def get_observation_probs(self, hypergraph, parameters, observations):
        return np.array(self.sampler.get_pairwise_observations_probabilities(hypergraph, parameters, observations))

    def get_observations_mean(self, parameters):
        _, _, *mu = parameters
        return self.get_mixture_mean_std_from_proportions(
                self.get_edgetype_probabilities(parameters), mu)[0]

    # Specific to poisson mixutre distributions
    def get_mixture_mean_std_from_proportions(self, proportions, means):
        _means, _proportions = np.array(means), np.array(proportions)
        mixture_mean = np.sum(_proportions*_means)
        mixture_std = mixture_mean + np.sum(_proportions*mixture_mean*mixture_mean) - mixture_mean*mixture_mean
        return mixture_mean, mixture_std

    def get_loglikelihood(self, hypergraph, parameters, observations):
        return self.sampler.get_loglikelihood(hypergraph, parameters, observations)


    def sample(self, observations, ground_truth, sampling_directory, mu1_smaller_mu2=True, verbose=2):
        erase_sample(sampling_directory)
        maximum_likelihood = None
        best_chain = None

        for chain in range(self.config["sampling", "chain number"]):
            if verbose == 1:
                print("Sampling chain", chain)

            chain_directory = os.path.join(sampling_directory, chain_directory_prefix+str(chain)) + "/"
            if not os.path.isdir(chain_directory):
                os.mkdir(chain_directory)

            initial_hypergraph, initial_parameters = self._get_initial_random_variables(ground_truth, observations, mu1_smaller_mu2)
            average_likelihood = self._sample_chain(initial_parameters, initial_hypergraph, observations, chain, chain_directory, verbose)

            if maximum_likelihood is None or average_likelihood > maximum_likelihood:
                maximum_likelihood = average_likelihood
                best_chain = chain

        if self.config["sampling", "keep only best chain"]:
            remove_all_chains_but(best_chain, sampling_directory)

    def _sample_chain(self, initial_parameters, initial_hypergraph, observations, chain, sampling_directory, verbose=2):
        sample = lambda: self.sampler.sample(
                observations     = observations.tolist(),
                parameters       = initial_parameters,
                hypergraph       = initial_hypergraph,
                chain            = chain,
                sample_size      = self.config["sampling", "sample size"],
                burnin           = self.config["sampling", "burnin"],
                output_directory = sampling_directory
            )

        with mute_output( stdout=(verbose<2) ):
            return sample()

    def sample_hypergraph_chain(self, observations, ground_truth, sampling_directory, mu1_smaller_mu2, iterations=[0, 1], points=100):
        initial_hypergraph, initial_parameters = self._get_initial_random_variables(ground_truth, observations, mu1_smaller_mu2)

        self.sampler.sample_hypergraph_chain(
                observations     = observations.tolist(),
                parameters       = initial_parameters,
                hypergraph       = initial_hypergraph,
                gibbs_iterations = list(map(int, iterations)),
                points           = points,
                mh_steps         = self.config["models", self.name, "mh maximum iterations"],
                output_directory = sampling_directory
            ),


    def _get_initial_random_variables(self, ground_truth, observations, mu1_smaller_mu2=True):

        if self.config["sampling", "use groundtruth"] and ground_truth is not None:
            initial_hypergraph, initial_parameters = ground_truth
        else:
            initial_hypergraph, initial_parameters = None, [None]*5

        if None in initial_parameters: # Replace unkown parameters by estimations
            estimated_parameters = self._get_adjusted_parameters_estimation(observations, mu1_smaller_mu2)
            for i, (known_value, estimated_value) in enumerate(zip(initial_parameters, estimated_parameters)):
                if known_value is None:
                    initial_parameters[i] = estimated_value if estimated_value > 0 else 1e-6

            if initial_hypergraph is None:
                initial_hypergraph = self._get_initial_hypergraph(observations)

        return self.adjust_hypergraph(initial_hypergraph.get_copy()), initial_parameters

    def _get_initial_hypergraph(self, observations):
        return pygrit.generate_hypergraph_from_adjacency(observations > 2)

    @abstractmethod
    def _get_adjusted_parameters_estimation(self, observations, mu1_smaller_mu2=True):
        pass

    def _estimate_initial_parameters(self, observations):
        values, occurences = get_occurences_in_observations(observations)

        def get_estimate():
            with warnings.catch_warnings():
                warnings.filterwarnings(action='ignore', category=RuntimeWarning)

                estimate = optimize.minimize(pygrit.get_neg_mixture_likelihood, x0=np.array([.8, .1, .1, .1, 10, 30]), bounds=np.array([(1e-8, 1e8)]*6),
                        constraints={"type":"eq", "fun": lambda x: np.sum(x[:3])-1}, tol=1e-2, args=(occurences.astype(np.float), values.astype(np.float)))

            return estimate.x, -estimate.fun


        best_parameters, best_likelihood = get_estimate()

        for i in range(49):
            parameters, likelihood = get_estimate()

            if likelihood > best_likelihood:
                best_parameters = parameters.copy()
                best_likelihood = likelihood

        index_order = np.argsort(best_parameters[3:])
        return { "rho": best_parameters[index_order], "mu": best_parameters[3+index_order] }


class PHG(InferenceModel):
    name = "phg"
    complete_name = "Hypergraph"
    correlated = True

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.with_correlation = True
        self.sampler = pygrit.PHG(
                    window_size = self.config["models", "phg", "window size"],
                    tolerance   = self.config["models", "phg", "tolerance"],
                    mh_minit    = self.config["models", "phg", "mh minimum iterations"],
                    mh_maxit    = self.config["models", "phg", "mh maximum iterations"],
                    eta         = self.config["models", "phg", "eta"],
                    chi_0       = self.config["models", "phg", "chi_0"],
                    chi_1       = self.config["models", "phg", "chi_1"],
                    model_hyperparameters = self.config["models", "phg", "hyperparameters"],
                    move_probabilities    = self.config["models", "phg", "move probabilities"],
                )

    def get_parameter_names(self):
        return "p", "q", "\\mu_0", "\\mu_1", "\\mu_2"

    def get_edgetype_probabilities(self, parameters):
        p, prob_pairwise_edge, *mu = parameters
        prob_pairwise_triangle = 1-p**(1/(self.n-2))
        prob_pairwise_no_hyperedge = (1-prob_pairwise_edge)*(1-prob_pairwise_triangle)
        return prob_pairwise_no_hyperedge, prob_pairwise_edge, prob_pairwise_triangle

    def _get_adjusted_parameters_estimation(self, observations, mu1_smaller_mu2):
        estimated_parameters = self._estimate_initial_parameters(observations)
        if mu1_smaller_mu2:
            hyperedge_proportions = [estimated_parameters["rho"][i] for i in range(3)]
            hyperedge_means       = [estimated_parameters["mu"][i] for i in range(3)]
        else:
            hyperedge_proportions = [estimated_parameters["rho"][i] for i in [0, 2, 1]]
            hyperedge_means       = [estimated_parameters["mu"][i] for i in [0, 2, 1]]

        p = (1-hyperedge_proportions[2])**(self.n-2)
        q = hyperedge_proportions[1]*(1-hyperedge_proportions[2])

        return [p, q] + hyperedge_means

    def adjust_hypergraph(self, hypergraph):
        return hypergraph


class PES(InferenceModel):
    name = "pes"
    complete_name = "Categorical model"
    correlated = False

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.with_correlation = False
        self.sampler = pygrit.PES(
                    window_size = self.config["models", "phg", "window size"],
                    tolerance   = self.config["models", "phg", "tolerance"],
                    mh_minit    = self.config["models", "phg", "mh minimum iterations"],
                    mh_maxit    = self.config["models", "phg", "mh maximum iterations"],
                    eta         = self.config["models", "phg", "eta"],
                    model_hyperparameters = self.config["models", "phg", "hyperparameters"],
                    move_probabilities    = self.config["models", "phg", "move probabilities"],
                )

    def get_parameter_names(self):
        return "q_1", "q_2", "\\mu_0", "\\mu_1", "\\mu_2"

    def get_edgetype_probabilities(self, parameters):
        q1, q2, *mu = parameters
        return (1-q1)*(1-q2), q1, q2

    def _get_adjusted_parameters_estimation(self, observations, mu1_smaller_mu2):
        estimated_parameters = self._estimate_initial_parameters(observations)

        q1 = estimated_parameters["rho"][1]
        q2 = estimated_parameters["rho"][2]
        hyperedge_means = [estimated_parameters["mu"][i] for i in range(3)]

        return [q1, q2] + hyperedge_means

    def adjust_hypergraph(self, hypergraph):
        return pygrit.project_hypergraph_on_graph(hypergraph)


class PER(InferenceModel):
    name = "per"
    complete_name = "Graph model"
    correlated = False

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.with_correlation = False
        self.sampler = pygrit.PER(
                    window_size = self.config["models", "phg", "window size"],
                    tolerance   = self.config["models", "phg", "tolerance"],
                    mh_minit    = self.config["models", "phg", "mh minimum iterations"],
                    mh_maxit    = self.config["models", "phg", "mh maximum iterations"],
                    eta         = self.config["models", "phg", "eta"],
                    model_hyperparameters = self.config["models", "phg", "hyperparameters"],
                    move_probabilities    = self.config["models", "phg", "move probabilities"],
                )

    def get_parameter_names(self):
        return "q_1", "q_2", "\\mu_0", "\\mu_1", "\\mu_2"

    def get_edgetype_probabilities(self, parameters):
        q1, q2, *mu = parameters
        return (1-q1), q1, 0

    def _get_adjusted_parameters_estimation(self, observations, mu1_smaller_mu2):
        estimated_parameters = self._estimate_initial_parameters(observations)

        q = estimated_parameters["rho"][1] + estimated_parameters["rho"][2]
        hyperedge_means = [estimated_parameters["mu"][i] for i in range(2)] + [0]

        return [q, 0] + hyperedge_means

    def adjust_hypergraph(self, hypergraph):
        return pygrit.project_hypergraph_on_graph(hypergraph)

models = { model.name: model for model in [PHG, PES, PER]}
