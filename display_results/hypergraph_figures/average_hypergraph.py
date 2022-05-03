import os, sys
import numpy as np
from matplotlib import pyplot

import pygrit
import drawing
sys.path.append("../")
sys.path.append("../..")
import plot_setup
from modelling.models import models
from utils.config import ConfigurationParserWithModels, get_config, get_dataset_name
from utils.output import get_output_directory_for, metrics_filename, get_sample_files, get_map_estimator
from generation.hypergraph_generation import load_binary_hypergraph
from generation.observations_generation import load_binary_observations

os.chdir("../..")


args = ConfigurationParserWithModels().parser.parse_args()

config = get_config(args)
dataset_name = get_dataset_name(args)

vertex_number = config["vertex number"]
sample_size = config["sampling", "sample size"]
vertex_positions = None

observations = load_binary_observations(dataset_name)
ground_truth = load_binary_hypergraph(dataset_name)
ground_truth_exists = ground_truth is not None
model_number = len(args.models)

inference_models = [models[model_name](config) for model_name in args.models]


if ground_truth_exists:
    fig, ax = pyplot.subplots(2, len(args.models)+(not args.o), figsize=( 4*(model_number+1)+1, 7))

    if args.g == "karate.json":
        vertex_positions = np.loadtxt("make_figures/hypergraph_figures/karate.pos")
    else:
        vertex_positions = drawing.find_vertex_positions(ground_truth, True)

    drawing.drawHyperedges(ground_truth, ax[0, 0], vertex_positions, "neutral")
    drawing.draw_vertices(ax[0, 0], vertex_positions)
    ax[0, 0].set_axis_off()
    ax[0, 0].set_title("Ground truth")

    drawing.drawHyperedges(ground_truth, ax[1,0], vertex_positions, "projection")
    drawing.draw_vertices(ax[1, 0], vertex_positions)
    ax[1, 0].set_axis_off()

else:
    fig, ax = pyplot.subplots(2, len(args.models), figsize=( 4*model_number+1, 4))


for i, model in enumerate(inference_models):
    sample_directory = get_output_directory_for("inference", dataset_name, model.name)
    hypergraph_files = [i[0] for i in get_sample_files(sample_directory, sample_size)]

    if model.correlated:
        average_hypergraph = pygrit.get_average_hypergraph(hypergraph_files)
    else:
        average_hypergraph = pygrit.get_average_hypergraph_edgestrength(hypergraph_files)

    if vertex_positions is None:
        vertex_positions = drawing.find_vertex_positions(average_hypergraph, not model.correlated)


    if ground_truth_exists:
        # drawing.drawHyperedgeDifference(pygrit.project_hypergraph_on_graph(average_hypergraph),
                                        # pygrit.project_hypergraph_on_graph(ground_truth),
                                        # differences_ax, vertex_positions, "per")
        map_ax = ax[1, i+1]
        average_ax = ax[0, i+1]

    else:
        map_ax = ax[1, i]
        average_ax = ax[i]

    map_hypergraph = get_map_estimator(sample_directory, sample_size, model, observations)[0]
    drawing.drawHyperedges(map_hypergraph, map_ax, vertex_positions, model.name)
    drawing.draw_vertices(map_ax, vertex_positions)

    drawing.draw_vertices(map_ax, vertex_positions)
    map_ax.set_axis_off()

    drawing.drawHyperedges(average_hypergraph, average_ax, vertex_positions, model.name)
    drawing.draw_vertices(average_ax, vertex_positions)
    average_ax.set_axis_off()
    average_ax.set_title(model.complete_name)

ymax = np.max(vertex_positions[:, 1])
xmin = np.min(vertex_positions[:, 0])
ax[0, 0].text(xmin, ymax, "Average", ha="right")
ax[1, 0].text(xmin, ymax, "MAP", ha="right")

figures_directory = plot_setup.get_figure_dir(dataset_name)
if not os.path.isdir(figures_directory):
    os.mkdir(figures_directory)

pyplot.axis("off")
pyplot.subplots_adjust(left=.05, right=1, bottom=0, top=.9, wspace=0.05, hspace=0.05)
pyplot.savefig(figures_directory+f"/{dataset_name}_average_hypergraph.pdf", bbox_inches='tight')
pyplot.show()
