import numpy as np
import scipy.interpolate as si
from matplotlib import patches, path
import itertools

import sys
import networkx
sys.path.append("../")
from plot_setup import model_colors


vertex_color = "#e8e8e8"
vertex_outercolor = "#696969"

edge_color = "#1f1f1f"
strong_edge_color = model_colors["pes"]
edge_width = 2.5
strong_edge_width = 2.5
triangle_sides_color = model_colors["phg"]
triangle_width = 1.5
triangle_color = model_colors["phg"]

edge_neg = "#922121"
edge_pos = "#2c40a3"
triangle_neg = edge_neg
triangle_pos = edge_pos
vertex_layout = networkx.drawing.layout.kamada_kawai_layout


def find_vertex_positions(hypergraph, with_correlation=True):
    n = hypergraph.get_size()

    if with_correlation:
        get_edge_type = lambda i, j: hypergraph.get_highest_order_hyperedge_with(i, j)
    else:
        get_edge_type = lambda i, j: hypergraph.get_edge_multiplicity(i, j)

    edgelist = [(i, j) for i in range(n)
                        for j in range(n) if get_edge_type(i, j)>0]

    nx_graph = networkx.from_edgelist(edgelist)
    nx_graph.add_nodes_from(range(0, n))
    vertex_positions = vertex_layout(nx_graph)

    return np.array([vertex_positions[i] for i in range(n)])


def draw_vertices(ax, vertex_positions):
    ax.scatter(vertex_positions[:, 0], vertex_positions[:, 1],
                edgecolor=vertex_outercolor, facecolor=vertex_color,
                s=125, linewidth=1.5, zorder=4)


def drawHyperedges(hypergraph, ax, node_positions, model):
    patch_list = []

    if model == "pes":
        edges = [[j for j, m in hypergraph.get_edges_from(i) if m==1 and i<j] for i in range(hypergraph.get_size())]
        patch_list.append(get_edge_patch(node_positions, edges, color=edge_color, alpha=.8, linewidth=edge_width, zorder=3))

        strong_edges = [[j for j, m in hypergraph.get_edges_from(i) if m==2 ] for i in range(hypergraph.get_size())]
        patch_list.append(get_edge_patch(node_positions, strong_edges, color=strong_edge_color, alpha=.7, linewidth=strong_edge_width, zorder=2))

    elif model in ["phg", "per", "neutral"]:
        edges = [[j for j, m in hypergraph.get_edges_from(i) if m==1 and i<j] for i in range(hypergraph.get_size())]
        patch_list.append(get_edge_patch(node_positions, edges, color=edge_color, alpha=.8, linewidth=edge_width, zorder=3))

        if model in ["phg", "neutral"]:
            facecolor = "gray" if model=="neutral" else triangle_color
            edgecolor = "gray" if model=="neutral" else triangle_sides_color
            triangles = hypergraph.get_full_triangle_list()
            patch_list += get_triangle_patches(node_positions, triangles, facecolor=facecolor, edgecolor=edgecolor, alpha=0.2, linewidth=triangle_width, zorder=2)

    elif model == "projection":
        edges = [list(x) for x in get_projected_edgelist(hypergraph)]
        patch_list = [get_edge_patch(node_positions, edges, color=edge_color, alpha=.8, linewidth=edge_width, zorder=3)]

    for p in patch_list:
        ax.add_patch(p)
    return patch_list

def drawHyperedgeDifference(hypergraph1, hypergraph2, ax, node_positions, model):
    patch_list = []

    if model in ["pes", "per"]:
        edges1 = get_projected_edgelist(hypergraph1)
        edges2 = get_projected_edgelist(hypergraph2)
        common_edges = [edges1[i].intersection(edges2[i]) for i in range(len(edges1))]
        extra_edges1 = [list(edges1[i].difference(common_edges[i])) for i in range(len(edges1))]
        extra_edges2 = [list(edges2[i].difference(common_edges[i])) for i in range(len(edges2))]

        patch_list.append(get_edge_patch(node_positions, extra_edges1, color=edge_pos, alpha=.8, linewidth=edge_width, zorder=3))
        patch_list.append(get_edge_patch(node_positions, extra_edges2, color=edge_neg, alpha=.8, linewidth=edge_width, zorder=3))

    elif model == "phg":
        triangles1 = hypergraph1.get_full_triangle_list()
        triangles2 = hypergraph2.get_full_triangle_list()
        extra_triangles1 = [t for t in triangles1 if t not in triangles2]
        extra_triangles2 = [t for t in triangles2 if t not in triangles1]

        patch_list += get_triangle_patches(node_positions, extra_triangles1, facecolor=triangle_pos, edgecolor="None", alpha=.2, zorder=2)
        patch_list += get_triangle_patches(node_positions, extra_triangles2, facecolor=triangle_neg, edgecolor="None", alpha=.2, zorder=2)


    for p in patch_list:
        ax.add_patch(p)
    return patch_list


def get_projected_edgelist(hypergraph):
    edges = [set([j for j, m in hypergraph.get_edges_from(i) if m>0]) for i in range(hypergraph.get_size())]

    for triplet in hypergraph.get_full_triangle_list():
        for i, j in itertools.combinations([triplet.i, triplet.j, triplet.k], 2):
            edges[i].add(j)

    ordered_edges = [set() for i in range(hypergraph.get_size())]
    for i in range(hypergraph.get_size()):
        for j in edges[i]:
            if i<j:
                ordered_edges[i].add(j)
            else:
                ordered_edges[j].add(i)
    return ordered_edges


def get_edge_patch(node_positions, adjacency_list, **kwargs):
    xpositions, ypositions = node_positions.T

    tmp = [[node_positions[i]]*len(neighbours) for i, neighbours in enumerate(adjacency_list)]
    draw_from = []
    for x in tmp:
        draw_from.extend(x)
    draw_from = np.array(draw_from)

    draw_to = np.array([node_positions[j] for neighbours in adjacency_list for j in neighbours])

    if draw_from.shape[0] == 0:
        return patches.Circle(node_positions[0], color="none", radius=1e-4)
    vertices = np.zeros((2*draw_from.shape[0], 2))
    vertices[::2] = draw_from
    vertices[1::2] = draw_to

    instructions = np.ones(2*draw_from.shape[0], int)*path.Path.LINETO
    instructions[::2] = path.Path.MOVETO

    edge_path = path.Path(vertices, instructions)
    return patches.PathPatch(edge_path, **kwargs)


def get_triangle_patches(node_positions, triangles, **kwargs):
    _patches = []

    for triplet in triangles:
        vertices = [ [node_positions[point][0], node_positions[point][1]]
                for point in [triplet.i, triplet.j, triplet.k] ]
        vertices = np.array(vertices + [vertices[0]])

        code = [path.Path.MOVETO] + [path.Path.LINETO]*3
        _path = path.Path(vertices, code)
        _patches.append(patches.PathPatch(_path, **kwargs))
    return _patches
