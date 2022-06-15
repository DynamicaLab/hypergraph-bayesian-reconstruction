import os, sys
import numpy as np
from matplotlib import pyplot

import pygrit
import drawing
sys.path.append("../")
sys.path.append("../..")
import plot_setup
from modeling.config import GenericConfigurationParser, get_config, get_dataset_name
from generation.hypergraph_generation import load_binary_hypergraph

os.chdir("../..")


args = GenericConfigurationParser().parser.parse_args()
config = get_config(args)
dataset_name = get_dataset_name(args)

ground_truth = load_binary_hypergraph(dataset_name)
if ground_truth is None:
    raise ValueError("No ground truth hypergraph for this dataset.")


vertex_number = config["vertex number"]
sample_size = config["sampling", "sample size"]
vertex_positions = None

if args.g == "karate.json":
    vertex_positions = np.loadtxt("display_results/hypergraph_figures/karate.pos")
else:
    vertex_positions = drawing.find_vertex_positions(ground_truth)

fig, ax = pyplot.subplots(figsize=(6, 4))
drawing.drawHyperedges(ground_truth, ax, vertex_positions, "phg")
drawing.draw_vertices(ax, vertex_positions)

fig.suptitle(f"Dataset {dataset_name} ground truth")
pyplot.axis("off")
pyplot.tight_layout()

pyplot.show()
