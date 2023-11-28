import os, sys
import re
import numpy as np
from matplotlib import pyplot
from tqdm import tqdm

import plot_setup
sys.path.append("../")
from modeling.models import models
from modeling.config import get_config_from_str
from modeling.output import get_output_directory_for, find_chains, get_sample_files_of_chain
from generation.hypergraph_generation import load_binary_hypergraph
from generation.observations_generation import load_binary_observations

import pygrit


def get_sample_number(file_names):
    expr = re.compile(r"hypergraph([0-9]+).*")
    match = expr.search(file_names[0])
    if match is not None:
        return int(match.group(1))
    else:
        return -1


os.chdir("../")

fig_pes, ax_pes = pyplot.subplots(2, 1, figsize=(6, 4))
fig_phg, ax_phg = pyplot.subplots(2, 1, figsize=(6, 4))

dataset_shortname = "fixed_worst"
for dataset_name, color in zip([dataset_shortname, dataset_shortname+"_gt"], [plot_setup.red, plot_setup.green]):
    config = get_config_from_str(f"{dataset_name}.json", "g")

    vertex_number = config["vertex number"]
    sample_size = config["sampling", "sample size"]
    vertex_positions = None

    inference_models = [models[model_name](config) for model_name in ["pes", "phg"]]
    observations = load_binary_observations(dataset_name)

    for model, ax in zip(inference_models, [ax_pes, ax_phg]):
        ground_truth = model.adjust_hypergraph(load_binary_hypergraph(dataset_name))
        sample_directory = get_output_directory_for("inference", dataset_name, model.name)

        if model.name == "phg":
            distance = pygrit.get_global_hamming_distance
        else:
            distance = pygrit.get_edge_hamming_distance

        for chain in find_chains(sample_directory):
            sample_files = list(get_sample_files_of_chain(chain, sample_directory, config["sampling", "sample size"]))
            sample_files.sort(key=get_sample_number)

            initial_structure, initial_parameters = model._get_initial_random_variables(
                    ground_truth=(ground_truth, [None]*5), observations=observations
                )
            print("initial distance", model.name, dataset_name, ":", distance(initial_structure, ground_truth))
            print("initial loglikelihood", model.name, dataset_name, ":", model.sampler.get_loglikelihood(initial_structure, initial_parameters, observations))
            distances = []
            likelihoods = []
            for hypergraph_file, parameters_file in sample_files:
                hypergraph = pygrit.Hypergraph.load_from_binary(hypergraph_file)
                parameters = np.fromfile(parameters_file, dtype=np.double)
                likelihoods.append( model.sampler.get_loglikelihood(hypergraph, parameters, observations) )
                distances.append(distance(hypergraph, ground_truth))

            label = "Ground truth initialization" if "gt" in dataset_name else "Heuristic initialization"
            iterations = np.arange(len(distances))
            ax[0].plot(iterations, distances, color=color,
                    label=label if chain==0 and model.name == "phg" else None)
            ax[1].plot(iterations, likelihoods, color=color)

        ax[0].set_ylabel("Distance to\nground truth")
        ax[1].set_ylabel("Loglikelihood")
        ax[1].set_xlabel("Posterior sample point")
        ax[0].set_title(model.complete_name)
        if model.name == "phg":
            ax[0].legend()
            ax[0].set_ylim(80, 102)

fig_pes.tight_layout()
fig_phg.tight_layout()
fig_phg.savefig(f"/home/simon/convergence_phg_{dataset_shortname}.png", dpi=200)
fig_pes.savefig(f"/home/simon/convergence_pes_{dataset_shortname}.png", dpi=200)
#pyplot.show()
