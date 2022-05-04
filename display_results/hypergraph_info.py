import os, sys
import numpy as np
from matplotlib import pyplot, colors
from numpy.lib.function_base import percentile

import plot_setup
sys.path.append("../")
from modeling.config import GenericConfigurationParser, get_dataset_name, get_config
from generation.hypergraph_generation import load_binary_hypergraph
import pygrit

os.chdir("../")


def count_triangles(A):
    return int( np.sum(np.diag(A@A@A))/6 )

args = GenericConfigurationParser().parser.parse_args()
config = get_config(args)
dataset_name = get_dataset_name(args)

groundtruth_hypergraph = load_binary_hypergraph(dataset_name)
n = groundtruth_hypergraph.get_size()

edge_types = np.array([[groundtruth_hypergraph.get_highest_order_hyperedge_with(i, j) for i in range(n)] for j in range(n)])
interactions = (edge_types>0).astype(np.int32)
_2edges = (edge_types==1).astype(np.int32)
_3edges = (edge_types==2).astype(np.int32)

print("Vertex number:", n)

edgetype_count = np.bincount(edge_types.ravel())/2
edgetype_count[0] -= n/2
print("Edge types count:", edgetype_count.astype(np.int32))
print("Edge types proportions:", [round(x*100, 4) for x in edgetype_count/np.sum(edgetype_count)], "%")

print(f"2-edges in triangle: {100*pygrit.count_edges_in_triangles(edge_types)/(np.sum(_2edges)/2):.4f} %")

_3edges_triangles = count_triangles(_3edges)
total_triangles = count_triangles(interactions)
print(f"Triangles with 2-edges: {100*(total_triangles-_3edges_triangles)/total_triangles:.4f} %")
