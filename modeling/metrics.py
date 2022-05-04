import numpy as np
from numpy.core.numeric import NaN
from numpy.lib.twodim_base import triu_indices

from .output import find_chains, get_sample_of_chain, get_edgetype_probabilities_of_chain, write_metrics
import pygrit


class ConfusionMatrix:
    name = "confusion matrix"

    def __init__(self, hypergraph_groundtruth, swap_edge_types):
        self.hypergraph_groundtruth = hypergraph_groundtruth
        self.swap_edge_types = swap_edge_types

        if hypergraph_groundtruth is not None:
            self.n = hypergraph_groundtruth.get_size()
        self.metric = []

    def compute_with(self, edgetype_probabilities, average_parameters):
        if self.hypergraph_groundtruth is None:
            return

        average_edgetype = np.argmax(edgetype_probabilities, 0)
        average_edgetype.resize((self.n, self.n))

        # Hypergraph data used always contains triangles. Otherwise change last flag to False
        self.metric.append(pygrit.get_confusion_matrix(self.hypergraph_groundtruth, average_edgetype.tolist(), with_correlation=True, edge_types_swapped=self.swap_edge_types))

    def get_metric(self):
        return self.metric


class PairwiseObservationsAverage:
    name = "pairwise average"

    def __init__(self, observations, inference_model):
        self.inference_model = inference_model
        self.observations_ordered_pairs = pygrit.get_decreasing_ordered_pairs(observations)
        self.n = observations.shape[0]
        self.metric = []

    def compute_with(self, edgetype_probabilities, parameters_average):
        prob0, prob1, prob2 = edgetype_probabilities
        n = self.n
        self.metric.append(
                [self.inference_model.get_mixture_mean_std_from_proportions([prob0[int(i)*n+int(j)], prob1[int(i)*n+int(j)], prob2[int(i)*n+int(j)]], parameters_average[2:])
                    for _, i, j in self.observations_ordered_pairs]
                )

    def get_metric(self):
        return self.metric

class DiscrepancyPValue:
    name = "discrepancy p-value"

    def __init__(self, observations, inference_model):
        self.non_zero_observations = observations[observations>0]
        self.inference_model = inference_model
        self.larger_posterior_discrepancy = 0
        self.N = 0

    def setup(self, hypergraph, parameters):
        self.observation_mean = self.inference_model.get_observations_mean(parameters)
        self.observed_discrepancy = np.sum(self.non_zero_observations*(np.log(self.non_zero_observations/self.observation_mean)))

    def compute_with(self, posterior_observations):
        non_zero_posterior = posterior_observations[posterior_observations>0]
        posterior_discrepancy = np.sum(non_zero_posterior*(np.log(non_zero_posterior/self.observation_mean)))
        self.larger_posterior_discrepancy += posterior_discrepancy > self.observed_discrepancy
        self.N += 1

    def get_metric(self):
        return NaN if self.N==0 else self.larger_posterior_discrepancy/self.N


def get_edgetypes(hypergraph, with_correlation):
    if with_correlation:
        get_edgetype = lambda i, j: hypergraph.get_highest_order_hyperedge_with(i, j)
    else:
        get_edgetype = lambda i, j: hypergraph.get_edge_multiplicity(i, j)

    return [ get_edgetype(i, j) for i, j in zip(*np.triu_indices(hypergraph.get_size(), k=1)) ]


def get_upper_triangular_array(arr):
    return arr[np.triu_indices_from(arr, k=1)]


class SumResiduals:
    name = "sum residuals"

    def __init__(self, observations, with_correlation=True, hypergraph_groundtruth=None):
        self.observations = observations
        self.metric = []
        self.true_edgetypes = None

        if hypergraph_groundtruth is not None:
            self.true_edgetypes = get_edgetypes(hypergraph_groundtruth, with_correlation)

    def setup(self, hypergraph, parameters):
        pass

    def compute_with(self, posterior_observations):
        if self.true_edgetypes is None:
            self.metric.append(np.sum(self.observations-posterior_observations))

        else:
            self.metric.append(
                pygrit.get_sum_residuals_of_types(self.true_edgetypes, self.observations, posterior_observations)
            )

    def get_metric(self):
        return np.array(self.metric).T


class SumAbsoluteResiduals:
    name = "sum absolute residuals"

    def __init__(self, observations, with_correlation=True, hypergraph_groundtruth=None):
        self.observations = observations
        self.metric = []
        self.true_edgetypes = None

        if hypergraph_groundtruth is not None:
            self.true_edgetypes = get_edgetypes(hypergraph_groundtruth, with_correlation)


    def setup(self, hypergraph, parameters):
        pass

    def compute_with(self, posterior_observations):
        if self.true_edgetypes is None:
            self.metric.append(np.sum(np.abs(self.observations-posterior_observations)))

        else:
            self.metric.append(
                pygrit.get_sum_absolute_residuals_of_types(self.true_edgetypes, self.observations, posterior_observations)
            )

    def get_metric(self):
        return np.array(self.metric).T


class WAIC:
    # Equations from http://www.stat.columbia.edu/~gelman/research/published/waic_understand3.pdf
    name = "WAIC"

    def __init__(self, observations, inference_model):
        self.inference_model = inference_model
        self.observations = observations
        self.aggregated_modelprobs = []
        self.n = observations.shape[0]

    def compute_with(self, hypergraph, parameters):
        self.aggregated_modelprobs.append(
                self.inference_model.get_observation_probs(hypergraph, parameters, self.observations)
            )

    def get_metric(self):
        self.aggregated_modelprobs = np.array(self.aggregated_modelprobs)
        llpd = np.sum(np.log(np.mean(self.aggregated_modelprobs, axis=1)))
        p_waic2 = np.sum(np.var(np.log(self.aggregated_modelprobs), axis=1)) * 2/(self.n*self.n-1)
        return 2*(p_waic2-llpd)


def compute_and_save_all_metrics(sample_directory, observations, hypergraph_groundtruth, sample_size, inference_model, observations_per_sample, swap_edge_types):
    metrics = {
            "sample_metrics": [
                    ConfusionMatrix(hypergraph_groundtruth, swap_edge_types),
                    PairwiseObservationsAverage(observations, inference_model)
                ],
            "sample_point_metrics": [
                    WAIC(observations, inference_model)
                ],
            "posterior_predictive_metrics": [
                    SumAbsoluteResiduals(observations, True, hypergraph_groundtruth),
                    SumResiduals(observations, True, hypergraph_groundtruth),
                    DiscrepancyPValue(observations, inference_model)
                ]
        }
    write_metrics(sample_directory,
            compute_metrics(sample_directory, sample_size, inference_model, observations_per_sample, metrics)
        )


def compute_and_save_tendency_metrics(sample_directory, observations, hypergraph_groundtruth, sample_size, inference_model, observations_per_sample, swap_edge_types):
    metrics = {
            "sample_metrics": [
                    ConfusionMatrix(hypergraph_groundtruth, swap_edge_types)
                ],
            "sample_point_metrics": [
                    WAIC(observations, inference_model)
                ],
            "posterior_predictive_metrics": [
                    SumAbsoluteResiduals(observations, True, hypergraph_groundtruth), # Using true because ground truth is hypergraph
                    SumResiduals(observations, True, hypergraph_groundtruth),
                    DiscrepancyPValue(observations, inference_model)
                ]
        }
    write_metrics(sample_directory,
            compute_metrics(sample_directory, sample_size, inference_model, observations_per_sample, metrics)
        )


def compute_metrics(sample_directory, sample_size, inference_model, observations_per_sample, metrics):

    def compute_metrics_with(metrics, *args):
        for metric in metrics:
            metric.compute_with(*args)


    for chain in find_chains(sample_directory):
        edgetype_probabilities = get_edgetype_probabilities_of_chain(chain, sample_directory, sample_size)
        average_parameters = np.zeros(5)
        actual_sample_size = 0


        for hypergraph, parameters in get_sample_of_chain(chain, sample_directory, sample_size):
            average_parameters += parameters
            actual_sample_size += 1

            compute_metrics_with(metrics["sample_point_metrics"], hypergraph, parameters)

            if metrics["posterior_predictive_metrics"]:
                for metric in metrics["posterior_predictive_metrics"]:
                    metric.setup(hypergraph, parameters)

                for i in range(observations_per_sample):
                    posterior_observations = inference_model.generate_observations(hypergraph, parameters)
                    compute_metrics_with(metrics["posterior_predictive_metrics"], posterior_observations)

        compute_metrics_with(metrics["sample_metrics"], edgetype_probabilities, average_parameters/actual_sample_size)

    return { metric.name: metric.get_metric() for metric_of_type in metrics.values() for metric in metric_of_type }
