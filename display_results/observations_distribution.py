import os, sys
import numpy as np
from matplotlib import pyplot, lines, patches, rcParams
from scipy import stats

import plot_setup
sys.path.append("../")
from modeling.models import models
from modeling.config import GenericConfigurationParser, get_dataset_name, get_config
from modeling.output import get_output_directory_for, observations_filename, get_sample_files
from generation.observations_generation import load_binary_observations


os.chdir("../")

#rcParams["axes.spines.left"] = False

class ConfigurationParserWithOptionalModels(GenericConfigurationParser):
    def __init__(self):
        super().__init__()
        self.parser.add_argument(type=str, dest="models",
                help="Models to process", nargs="*", default=[])
        self.parser.add_argument("--matrix", help="Show observation matrix", action="store_true")

args = ConfigurationParserWithOptionalModels().parser.parse_args()
dataset_name = get_dataset_name(args)
config = get_config(args)

inference_models = [models[model_name](config) for model_name in args.models]
sample_size = config["sampling", "sample size"]


observations = load_binary_observations(dataset_name)


figure_output_dir = plot_setup.get_figure_dir(dataset_name)
if not os.path.isdir(figure_output_dir):
    os.mkdir(figure_output_dir)

if args.matrix and args.o == "dolphins.json":
    pyplot.imshow(observations, cmap="binary", origin="lower")
    pyplot.colorbar()
    pyplot.ylabel("Dolphin")
    pyplot.xlabel("Dolphin")
    pyplot.savefig(figure_output_dir+f"/dolphins_matrix.svg", bbox_inches='tight')
    pyplot.show()
    exit()

pyplot.figure(figsize=(6, 5))
bins = np.arange(0, np.max(observations))-.5
heights = pyplot.hist(observations.flatten(), bins=bins, density=True, color="#EBEBEB",
                      edgecolor="#8e8e8e" if inference_models==[] else None)[0]
xvalues = np.arange(0, bins[-1])

linestyles = ['-', "-.", ":"]

for model, ls in zip(inference_models, linestyles):
    parameters_mean = None

    sample_location = get_output_directory_for("inference", dataset_name, model.name)
    for hypergraph_file, parameters_file in get_sample_files(sample_location, sample_size):
        if parameters_mean is None:
            parameters_mean = np.fromfile(parameters_file, dtype=np.double)
        else:
            parameters_mean += np.fromfile(parameters_file, dtype=np.double)


    if parameters_mean is not None:
        parameters_mean = parameters_mean/sample_size
        observation_parameters = parameters_mean[2:]

        for mean, edge_prob, color in zip(observation_parameters, model.get_edgetype_probabilities(parameters_mean), plot_setup.edgetype_colors):
            if edge_prob > 0:
                pyplot.plot(edge_prob*stats.poisson.pmf(xvalues, mean),
                            color=color, ls=ls, lw=2)

if inference_models != []:
    legend = [[], []]
    legend[0].append(patches.Patch(color=plot_setup.lightgray))
    legend[1].append("Observations")

    for i, hyperedge in enumerate(["No hyperedge", "Edge/Weak edge", "Triangle/Strong edge"]):
        legend[0].append(lines.Line2D([], [], linestyle='-', color=plot_setup.edgetype_colors[i]))
        legend[1].append(hyperedge)

    for model, ls in zip(inference_models, linestyles):
        legend[0].append(lines.Line2D([], [], linestyle=ls, color='gray'))
        legend[1].append(model.complete_name)
    pyplot.legend(*legend)

pyplot.xlabel("Pairwise observations $x_{ij}$")
# pyplot.ylabel(r"$P(x_{ij}| \mu_k)$")

#if heights[0] > 0.9:
pyplot.yscale("log")
#pyplot.ylim((1e-5, 1))

#pyplot.yticks([])
pyplot.tight_layout()

pyplot.savefig(figure_output_dir+f"/observations.svg", bbox_inches='tight')
pyplot.show()
