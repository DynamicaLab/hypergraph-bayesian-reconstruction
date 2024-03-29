from itertools import combinations
import numpy as np
import os

import pygrit
from modeling.output import get_output_directory_for, hypergraph_filename
from modeling.config import InvalidConfiguration


def load_binary_hypergraph(dataset_name):
    hypergraph_path = os.path.join(get_output_directory_for("data", dataset_name), hypergraph_filename)

    if os.path.isfile(hypergraph_path+"_edges") and os.path.isfile(hypergraph_path+"_triangles"):
        return pygrit.Hypergraph.load_from_binary(hypergraph_path)


def load_csv_hypergraph(filename, remove_disconnected_vertices=False, sep=", "):
    with open(filename, 'r') as file_stream:

        first_line = True
        for line in file_stream.readlines():
            if first_line:
                first_line = False
                if line.startswith("size="):
                    size = int(line.rstrip()[len("size="):])
                    hypergraph = pygrit.Hypergraph(size)
                    continue
                else:
                    hypergraph = pygrit.Hypergraph(3)

            vertices = [int(i) for i in line.split(sep)]

            required_size = max(vertices)+1
            if required_size > hypergraph.get_size():
                hypergraph.resize(required_size)

            if len(vertices) == 2:
                hypergraph.add_edge(*vertices)
            else:
                for comb in combinations(vertices, 3):
                    hypergraph.add_triangle(*comb)

    if remove_disconnected_vertices:
        return pygrit.remove_disconnected_vertices(hypergraph)
    return hypergraph


def write_hypergraph_to_binary(hypergraph, dataset_name):
    hypergraph.write_to_binary(os.path.join(get_output_directory_for("data", dataset_name),
        hypergraph_filename))


def transfer_hypergraph_data_to_binary(config, dataset_name):
    hypergraph = load_csv_hypergraph(config["dataset"], config["synthetic generation", "remove disconnected vertices"],
                                        sep=config["sep"])
    write_hypergraph_to_binary(hypergraph, dataset_name)
    return hypergraph


def verify_is_probability(p):
    if not(0 <= p <= 1):
        raise InvalidConfiguration("Incorrect hypergraph generation parameters. Probabilities are not in [0, 1]")


def generate_hypergraph(hypergraph_size, hypergraph_process, hypergraph_parameters):

    if hypergraph_process == "independent hyperedges":
        if not isinstance(hypergraph_parameters, list) or len(hypergraph_parameters) != 2:
            raise InvalidConfiguration("Incorrect hypergraph parameters in configuration file. "
                    f"Hypergraph model \"{hypergraph_process}\" requires a list of 2 floats")

        verify_is_probability(hypergraph_parameters[0])
        verify_is_probability(hypergraph_parameters[1])

        return pygrit.generate_independent_hyperedges_hypergraph(hypergraph_size, *hypergraph_parameters)
    elif hypergraph_process == "independent layered edges":
        if not isinstance(hypergraph_parameters, list) or len(hypergraph_parameters) != 2:
            raise InvalidConfiguration("Incorrect hypergraph parameters in configuration file. "
                    f"Hypergraph model \"{hypergraph_process}\" requires a list of 2 floats")

        if not(0 <= hypergraph_parameters[0] <= 1) or not(0 <= hypergraph_parameters[1] <= 1):
            raise InvalidConfiguration("Incorrect hypergraph parameters. "
                    "Probabilities are not in [0, 1]")

        return pygrit.generate_independent_layered_edges_hypergraph(hypergraph_size, *hypergraph_parameters)

    elif hypergraph_process == "beta-model":
        if not isinstance(hypergraph_parameters, list) or len(hypergraph_parameters) != 4:
            raise InvalidConfiguration("Incorrect hypergraph parameters. "
                    f"Hypergraph model \"{hypergraph_process}\" requires the following format: "
                    "\"[2-edge propensity average], [2-edge propensity std], "
                    "\"[3-edge propensity average], [3-edge propensity std]]\"")
        return pygrit.generate_beta_model_hypergraph_normal(hypergraph_size, *hypergraph_parameters)

    elif hypergraph_process == "miller CM":
        if not isinstance(hypergraph_parameters, list) or len(hypergraph_parameters) != 2:
            raise InvalidConfiguration("Incorrect hypergraph parameters. "
                    f"Hypergraph model \"{hypergraph_process}\" requires the following format: "
                    "\"[average 2-edge degree, average 3-edge degree]\"")

        p_edge = 1/hypergraph_parameters[0] if hypergraph_parameters[0]>0 else 0
        p_triangle = 1/hypergraph_parameters[1] if hypergraph_parameters[1]>0 else 0

        return pygrit.generate_miller_cm_hypergraph_geometric(hypergraph_size, p_edge, p_triangle)

    elif hypergraph_process == "hypergraph SBM":
        if not isinstance(hypergraph_parameters, list) or len(hypergraph_parameters) != 5:
            raise InvalidConfiguration("Incorrect hypergraph parameters. "
                    f"Hypergraph model \"{hypergraph_process}\" requires the following format: "
                    "\"[[community sizes], [P(2-edge in community i)], P(2-edge outside community), "
                    "[P(3-edge in community i)], P(3-edge outside community)]\"")

        if not isinstance(hypergraph_parameters[0], list) or sum(hypergraph_parameters[0]) != hypergraph_size:
            raise InvalidConfiguration("Incorrect hypergraph parameters. "
                    f"Community sizes for SBM don't sum to vertex number.")

        return pygrit.generate_sbm_hypergraph(*hypergraph_parameters)

    elif hypergraph_process == "only 3-cycles":
        if not isinstance(hypergraph_parameters, list) or len(hypergraph_parameters) != 2:
            raise InvalidConfiguration("Incorrect hypergraph parameters in configuration file. "
                    f"Hypergraph model \"{hypergraph_process}\" requires a list of a integer and a float.")

        verify_is_probability(hypergraph_parameters[1])

        return pygrit.generate_independent_hyperedges_only_cycles(hypergraph_size, *hypergraph_parameters)

    elif hypergraph_process == "no 3-cycles":
        if not isinstance(hypergraph_parameters, list) or len(hypergraph_parameters) != 2:
            raise InvalidConfiguration("Incorrect hypergraph parameters in configuration file. "
                    f"Hypergraph model \"{hypergraph_process}\" requires a list of 2 floats.")

        verify_is_probability(hypergraph_parameters[0])
        verify_is_probability(hypergraph_parameters[1])

        return pygrit.generate_independent_hyperedges_no_cycles(hypergraph_size, *hypergraph_parameters)

    raise InvalidConfiguration(f"Unknown hypergraph process \"{hypergraph_process}\".")


def generate_and_write_hypergraph(config, dataset_name):
    hypergraph = generate_hypergraph(config["vertex number"],
            hypergraph_process = config["synthetic generation", "hypergraph process"],
            hypergraph_parameters = config["synthetic generation", "hypergraph parameters"])

    write_hypergraph_to_binary(hypergraph, dataset_name)
    return hypergraph
