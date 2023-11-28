import os, sys
import numpy as np
from matplotlib import pyplot, rcParams

sys.path.append("../")
from modeling.config import ConfigurationParserWithModels, get_dataset_name, get_config
from modeling.output import (
        get_output_directory_for, find_chains,
        get_edgetype_probabilities_of_chain, get_sample_of_chain
    )
from modeling.models import models
from generation.observations_generation import load_binary_observations

import plot_setup


os.chdir("../")


args = ConfigurationParserWithModels().parser.parse_args()
config = get_config(args)

dataset_name = get_dataset_name(args)
output_directory = get_output_directory_for("data", dataset_name)
inference_models = [models[model_name](config) for model_name in args.models]

observations = load_binary_observations(dataset_name)

models_posterior_observations = {}
for model in inference_models:
    sample_directory = get_output_directory_for("inference", dataset_name, model.name) + "/"
    sample_size = config["sampling", "sample size"]

    average_parameters = np.zeros(5)
    actual_sample_size = 0
    for chain in find_chains(sample_directory):
        for hypergraph, parameters in get_sample_of_chain(chain, sample_directory, sample_size):
            average_parameters += parameters
            actual_sample_size += 1
    average_parameters /= actual_sample_size

    n = hypergraph.get_size()
    models_posterior_observations[model.name] = {"std": {}, "mean": {}}
    for chain in find_chains(sample_directory):
        prob0, prob1, prob2 = get_edgetype_probabilities_of_chain(chain, sample_directory, sample_size)

        for i, j in zip(*np.triu_indices(n, 1)):
            x_ij = observations[i, j]
            mean, std = model.get_mixture_mean_std_from_proportions(
                [prob0[int(i)*n+int(j)], prob1[int(i)*n+int(j)], prob2[int(i)*n+int(j)]],
                average_parameters[2:]
            )
            if models_posterior_observations[model.name]["std"].get(x_ij) is None:
                models_posterior_observations[model.name]["mean"][x_ij] = []
                models_posterior_observations[model.name]["std"][x_ij] = []
            models_posterior_observations[model.name]["mean"][x_ij].append(mean)
            models_posterior_observations[model.name]["std"][x_ij].append(std)


pyplot.figure(figsize=(plot_setup.fig_width, 2*plot_setup.fig_height))

model_number = len(models_posterior_observations)
for i, (model_name, posterior_values) in enumerate(models_posterior_observations.items()):
    color = plot_setup.model_colors[model_name]
    marker = plot_setup.model_markers[model_name]

    for j, value in enumerate(posterior_values["mean"].keys()):
        n = len(posterior_values["mean"][value])
        pyplot.errorbar([value]*n, posterior_values["mean"][value],
                        yerr=np.sqrt(posterior_values["std"][value]),
                        label=models[model_name].complete_name if j==0 else None,
                        color=color, markersize=5, marker=marker)

max_observation = np.max(observations)
pyplot.plot([0, max_observation], [0, max_observation], ls="--", color=plot_setup.lightgray)


figure_output_dir = plot_setup.get_figure_dir(dataset_name)
if not os.path.isdir(figure_output_dir):
    os.mkdir(figure_output_dir)

pyplot.xlabel("Pairwise observations $x_{ij}$")
pyplot.ylabel("Predicted\nobservations")
pyplot.xlim(0, None)
pyplot.ylim(0, None)
pyplot.legend()
pyplot.tight_layout()
pyplot.savefig(os.path.join(figure_output_dir, f"{dataset_name}_posterior_map_histogram.pdf"))
pyplot.savefig(os.path.join(figure_output_dir, f"{dataset_name}_posterior_map_histogram.png"), dpi=200)
#pyplot.show()
