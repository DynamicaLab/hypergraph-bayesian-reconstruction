import os, sys
import numpy as np
from matplotlib import pyplot

import pygrit
import drawing
sys.path.append("../")
sys.path.append("../..")
import plot_setup
from modeling.models import models
from modeling.config import get_config_from_str
from modeling.output import get_output_directory_for, metrics_filename, get_sample_files, get_map_estimator
from generation.hypergraph_generation import load_binary_hypergraph
from generation.observations_generation import load_binary_observations

os.chdir("../..")


dataset_name = "fixed_worst"
config = get_config_from_str(f"{dataset_name}.json", "g")

vertex_number = config["vertex number"]
sample_size = config["sampling", "sample size"]
vertex_positions = None

observations = load_binary_observations(dataset_name)
ground_truth = load_binary_hypergraph(dataset_name)
ground_truth_exists = ground_truth is not None

vertex_positions = drawing.find_vertex_positions(ground_truth, True)

for model_name in ["phg", "pes"]:
    fig, ax = pyplot.subplots(1, 1, figsize=(6, 4))

    drawing.draw_vertices(ax, vertex_positions)

    sample_directory1 = get_output_directory_for("inference", dataset_name, model_name)
    sample_directory2 = get_output_directory_for("inference", dataset_name+"_gt", model_name)
    map_hypergraph1 = get_map_estimator(sample_directory1, sample_size, models[model_name](config), observations)[0]
    map_hypergraph2 = get_map_estimator(sample_directory2, sample_size, models[model_name](config), observations)[0]

    #drawing.drawHyperedges(map_hypergraph1, model=model_name, node_positions=vertex_positions, ax=ax)
    drawing.drawHyperedgeDifference(map_hypergraph1, map_hypergraph2, ax, vertex_positions, models[model_name])

    pyplot.axis("off")
    pyplot.subplots_adjust(left=.06, right=1, bottom=0, top=1, wspace=0.07, hspace=0.2)
    pyplot.show()
