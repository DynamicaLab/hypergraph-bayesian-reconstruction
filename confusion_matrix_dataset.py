import os
import shutil
import numpy as np
import itertools
import logging

import graph_tool.all as gt
import graph_tool.inference as gt_inf
from cmdstanpy import CmdStanModel

from generation.hypergraph_generation import load_binary_hypergraph
from generation.observations_generation import generate_observations
from modeling.metrics import ConfusionMatrix, compute_metrics
from modeling.models import models, PES, PHG
from modeling.config import ConfigurationParserWithModels, get_config, get_dataset_name
from modeling.output import (
        create_output_directories, get_output_directory_for,
        write_metrics
    )


def estimate_pes_parameters(hypergraph):
    n = hypergraph.get_size()
    counts = [0, 0, 0]
    for i in range(n-1):
        for j in range(i+1, n):
            counts[hypergraph.get_highest_order_hyperedge_with(i, j)] += 1

    for i in range(3):
        counts[i] *= 2/(n*(n-1))
    return counts


def find_threshold(mu1, mu2, w1, w2):
    return int(round(1/(np.log(mu2)-np.log(mu1))*(mu2-mu1-np.log(w2)+np.log(w1))))


def compute_confusion_matrix(edgetypes, hypergraph):
    n = hypergraph.get_size()

    confusion_matrix = np.zeros((3, 3))
    for i in range(n-1):
        for j in range(i+1, n):
            confusion_matrix[
                    edgetypes[i, j],
                    hypergraph.get_highest_order_hyperedge_with(i, j)
                ] += 1
    return confusion_matrix


def estimate_average_edge_types_from_graph(adjacency_matrix, sample_size=100):
    graph = gt.Graph(directed=False)
    graph.add_edge_list(np.transpose(adjacency_matrix.nonzero()))
    state = gt_inf.CliqueState(graph)

    count_type2 = np.zeros_like(adjacency_matrix)

    for i in range(sample_size):
        state.mcmc_sweep(niter=50000)

        for v in state.f.vertices():      # iterate through factor graph
            if state.is_fac[v]:           # skip over factors
                continue
            # Verify clique occupation and is hyperedge
            if state.x[v] == 1 and len(state.c[v])>=3:
                for u, v in itertools.combinations(state.c[v], 2):
                    count_type2[u, v] += 1
                    count_type2[v, u] += 1

    edgetypes = np.copy(adjacency_matrix)
    edgetypes += count_type2>=(.5*sample_size)

    return edgetypes


def estimate_edge_prob(observations):
    data={
        "n":observations.shape[0],
        "X":observations.astype(np.int32)
    }

    file_dir_path = os.path.abspath(os.path.dirname(os.path.realpath(__file__)))

    logger = logging.getLogger("cmdstanpy")
    logger.disabled = True

    model = CmdStanModel(stan_file=os.path.join(file_dir_path, "stan-models", "undir.stan"))

    fit = model.sample(data=data, iter_sampling=200, iter_warmup=100, show_progress=False)
    return np.average(fit.Q, axis=0)


if __name__ == "__main__":
    observation_parameters = [0.01, 40, 50]

    args = ConfigurationParserWithModels().parse_args()

    config = get_config(args)
    dataset_name = get_dataset_name(args)
    inference_models = [models[model_name](config) for model_name in args.models]

    create_output_directories(dataset_name, args.models)

    hypergraph = load_binary_hypergraph(dataset_name)
    if hypergraph is None and not args.o:
        raise RuntimeError("No hypergraph binary file found. Run \"generate_data.py\" before sampling.")


    output_directory = os.path.join(get_output_directory_for("data", dataset_name))
    sampling_directory = os.path.join(output_directory, "tmp"+dataset_name)
    if not os.path.isdir(sampling_directory):
        os.mkdir(sampling_directory)



    ordered_mu = np.sort(observation_parameters)
    q = estimate_pes_parameters(hypergraph)

    threshold1 = find_threshold(*ordered_mu[:2], q[0], q[1])
    threshold2 = find_threshold(*ordered_mu[1:], q[1], q[2])


    confusion_matrices = {approach_name: []
                    for approach_name in ["threshold", "edge-threshold", "2step-bayesian"]
                                        + [model.name for model in inference_models]}
    repetitions = 10
    for i in range(repetitions):
        observations = generate_observations(hypergraph, config["synthetic generation", "observation process"],
                                                observation_parameters, True)


        edge_probs = estimate_edge_prob(observations)
        average_edgetypes = estimate_average_edge_types_from_graph( (edge_probs>0.5).astype(np.int32) )
        confusion_matrices["2step-bayesian"].append(compute_confusion_matrix(average_edgetypes, hypergraph))

        threshold_edgetypes = (observations>=threshold1).astype(np.int32)
        average_edgetypes = estimate_average_edge_types_from_graph(threshold_edgetypes)
        confusion_matrices["edge-threshold"].append(compute_confusion_matrix(average_edgetypes, hypergraph))

        threshold_edgetypes += (observations>=threshold2).astype(np.int32)
        confusion_matrices["threshold"].append(compute_confusion_matrix(threshold_edgetypes, hypergraph))

        for model in inference_models:
            print(f"{model.complete_name} {i+1}/{repetitions}")
            groundtruth = (hypergraph,
                    [None]*2+observation_parameters if model.name == PHG.name\
                    else [None]*2+[observation_parameters[0], min(observation_parameters[1:]), max(observation_parameters[1:])]
                    )

            model.sample(observations, ground_truth=groundtruth, sampling_directory=sampling_directory,
                         mu1_smaller_mu2=observation_parameters[1]<observation_parameters[2], verbose=0)

            swap_edge_types = observation_parameters[1]>observation_parameters[2] and model.name == PES.name

            metrics = compute_metrics(sampling_directory, config["sampling", "sample size"], model,
                        config["metrics", "generated observations number"],
                        { "sample_metrics": [ ConfusionMatrix(hypergraph, swap_edge_types) ],
                          "sample_point_metrics": [], "posterior_predictive_metrics": [] }
                    )
            confusion_matrices[model.name].append(metrics[ConfusionMatrix.name])
    write_metrics(output_directory, confusion_matrices)
    shutil.rmtree(sampling_directory)
