import os, sys
import numpy as np
from matplotlib import pyplot, animation

import pygrit
import drawing
sys.path.append("../")
sys.path.append("../..")
import plot_setup
from modeling.models import models
from modeling.config import ConfigurationParserWithModels, get_config, get_dataset_name
from modeling.output import get_output_directory_for, metrics_filename, get_sample
from generation.hypergraph_generation import load_binary_hypergraph


def get_sample_generators(dataset_name, sample_size):
    sample_generators = []
    for model in args.models:
        sample_generators.append( get_sample(get_output_directory_for("inference", dataset_name, model), sample_size) )
    return sample_generators


os.chdir("../..")


args = ConfigurationParserWithModels().parser.parse_args()

config = get_config(args)
dataset_name = get_dataset_name(args)
inference_models = [models[model_name](config) for model_name in args.models]

vertex_number = config["vertex number"]
sample_size = config["sampling", "sample size"]
vertex_positions = None

ground_truth = load_binary_hypergraph(dataset_name)
ground_truth_exists = ground_truth is not None
model_number = len(args.models)


if ground_truth_exists:
    fig, axes = pyplot.subplots(1, 1+len(args.models), figsize=( 4*(model_number+1)+1, 4))

    if args.g == "karate.json":
        vertex_positions = np.loadtxt("display_results/hypergraph_figures/karate.pos")
    else:
        vertex_positions = drawing.find_vertex_positions(ground_truth, True)

    drawing.drawHyperedges(ground_truth, axes[0], vertex_positions, "neutral")
    drawing.draw_vertices(axes[0], vertex_positions)
    axes[0].set_axis_off()
    axes[0].set_title("Ground truth")

else:
    fig, axes = pyplot.subplots(1, len(args.models), figsize=( 4*model_number+1, 4))
    if model_number == 1:
        axes = [axes]


generators = get_sample_generators(dataset_name, sample_size)
# Plot first graph, plot vertices and find vertex position if needed

for i, model in enumerate(inference_models):
    initial_graph = next(generators[i])[0]
    ax = axes[i+ground_truth_exists]

    if vertex_positions is None:
        vertex_positions = drawing.find_vertex_positions(initial_graph, model.with_correlation)

    drawing.drawHyperedges(initial_graph, ax, vertex_positions, model.name)
    drawing.draw_vertices(ax, vertex_positions)
    ax.set_axis_off()
    ax.set_title(f"{model.complete_name} posterior sample")

sample_id = 0
sample_id_text = axes[0].text(0.1, 0.9, f"$i={sample_id}$",
     horizontalalignment='center',
     verticalalignment='center',
     transform = axes[0].transAxes)

def update(frame):
    global generators, sample_id, axes
    sample_id += 1

    patches = []
    for i, model in enumerate(args.models):
        try:
            current_hypergraph = next(generators[i])[0]
        except StopIteration:
            generators = get_sample_generators(dataset_name, sample_size)
            current_hypergraph = next(generators[i])[0]
            sample_id = 1

        ax = axes[i+ground_truth_exists]
        ax.patches.clear()

        drawing.drawHyperedges(current_hypergraph, ax, vertex_positions, model)

    sample_id_text.set_text(f"$i={sample_id}$")


anim = animation.FuncAnimation(fig, update, frames=100, interval=250, save_count=30)
pyplot.tight_layout()
pyplot.show()
