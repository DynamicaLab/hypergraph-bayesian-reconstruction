import os, sys
import numpy as np
from matplotlib import pyplot, rcParams

sys.path.append("../")
from modeling.config import ConfigurationParserWithModels, get_dataset_name, get_config
from modeling.output import get_output_directory_for, get_map_estimator
from modeling.metrics import PosteriorObervationsCounts, compute_metrics
from modeling.models import models
from generation.observations_generation import load_binary_observations

import plot_setup


def merge_bins(bin_counts, bin_size):
    size = np.max(list(bin_counts.keys()))
    new_size = int(size/bin_size)

    merged_counts = {}

    for bin, count in bin_counts.items():
        new_bin = bin // bin_size
        if merged_counts.get(new_bin) is None:
            merged_counts[new_bin] = 0
        merged_counts[new_bin] += count

    return merged_counts

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

    hypergraph, parameters = get_map_estimator(sample_directory, config["sampling", "sample size"], model, observations)

    observation_bin_counts = PosteriorObervationsCounts()
    for i in range(10):
        observation_bin_counts.compute_with(model.generate_observations(hypergraph, parameters))
    models_posterior_observations[model.name] = observation_bin_counts.get_metric()



unmerged_bins_number = np.max(observations)+1
bin_number = max(np.ceil(np.log2(unmerged_bins_number)+1), 20)
bin_size = int(unmerged_bins_number/bin_number)

binshift = bin_size/2

bins = np.arange(0, unmerged_bins_number, bin_size)

pyplot.hist(
    observations.ravel(), bins=bins,
    alpha=0.4, color="gray", edgecolor=plot_setup.midblack,
    density=True,
    label="Original observations"
)

model_number = len(models_posterior_observations)
for i, (model_name, posterior_bin_counts) in enumerate(models_posterior_observations.items()):
    color = plot_setup.model_colors[model_name]

    width = bin_size/(model_number+1)

    merged_bins = merge_bins(posterior_bin_counts, bin_size)
    bincount_array = np.array([merged_bins.get(key, 0) for key in merged_bins])
    pyplot.bar(
        np.arange(0, len(bincount_array)*bin_size, bin_size)+i*width+binshift,
        bincount_array/np.sum(bincount_array),
        width=width, color=color,
        label=models[model_name].complete_name
    )


figure_output_dir = plot_setup.get_figure_dir(dataset_name)
if not os.path.isdir(figure_output_dir):
    os.mkdir(figure_output_dir)

pyplot.ylabel("Frequency")
pyplot.xlabel("Pairwise observations $x_{ij}$")
pyplot.yscale("log")
pyplot.xlim(0, None)
pyplot.legend()
pyplot.tight_layout()
pyplot.savefig(os.path.join(figure_output_dir, "posterior_map_histogram.pdf"))
#pyplot.show()
