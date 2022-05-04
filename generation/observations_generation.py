import numpy as np
import os
from scipy import stats

import pygrit
from modeling.output import get_output_directory_for, observations_filename
from modeling.config import InvalidConfiguration


def load_binary_observations(dataset_name):
    observations_path = os.path.join(get_output_directory_for("data", dataset_name), observations_filename)

    if os.path.isfile(observations_path):
        return np.load(observations_path)


def load_csv_matrix_observations(filename):
    return np.genfromtxt(filename, delimiter=",").astype(np.int64)

def load_timeseries_observations(filename, vertex_number, delimiter=","):
    observations = np.zeros((vertex_number, vertex_number), dtype=np.int64)

    with open(filename, "r") as file_stream:
        for line in file_stream.readlines():
            values = line.split(" ")[:2]
            i, j = list(map(int, values))
            observations[i][j] += 1
            observations[j][i] += 1
    return observations

def load_weighted_timeseries_observations(filename, vertex_number, delimiter=","):
    observations = np.zeros((vertex_number, vertex_number), dtype=np.int64)

    with open(filename, "r") as file_stream:
        for line in file_stream.readlines():
            values = line.split(" ")[:3]
            i, j, freq = list(map(int, values))
            observations[i][j] += freq
            observations[j][i] += freq
    return observations


def load_csv_edgelist_observations(filename, vertex_number, delimiter=","):
    observations = np.zeros((vertex_number, vertex_number))

    with open(filename, 'r') as file_stream:
        for line in file_stream.readlines():
            values = line.split(delimiter)[:3]
            i, j, xij = list(map(int, values))
            observations[i, j] = xij
            observations[j, i] = xij
    return observations.astype(np.int64)

def write_observations_to_binary(observations, dataset_name):
    np.save(os.path.join(get_output_directory_for("data", dataset_name), observations_filename),
            observations)


def transfer_observation_data_to_binary(config, dataset_name):
    if config["data format"] == "csv matrix":
        observations = load_csv_matrix_observations(config["dataset"])
    elif config["data format"] == "csv edgelist":
        observations = load_csv_edgelist_observations(config["dataset"], config["vertex number"])
    elif config["data format"] == "csv timeseries":
        observations = load_timeseries_observations(config["dataset"], config["vertex number"])
    elif config["data format"] == "csv weighted timeseries":
        observations = load_weighted_timeseries_observations(config["dataset"], config["vertex number"])
    else:
        raise InvalidConfiguration("Dataset format \""+config["format"]+"\" is not supported.")

    write_observations_to_binary(observations, dataset_name)
    return observations


def generate_observations(hypergraph, observation_process, observation_parameters, with_correlation):
    if observation_process == "poisson dominant hyperedge":
        if not isinstance(observation_parameters, list) or len(observation_parameters) != 3:
            raise InvalidConfiguration("Incorrect observation parameters. "
                    f"Observation process \"{observation_process}\" needs to be a list of 3 floats.")

        return pygrit.generate_poisson_observations(hypergraph, *observation_parameters, with_correlation)
    raise InvalidConfiguration(f"Unknown observation process \"{observation_process}\".")


def generate_and_write_observations(hypergraph, config, dataset_name, with_correlation):
    observations = generate_observations(hypergraph,
                        config["synthetic generation", "observation process"],
                        config["synthetic generation", "observation parameters"],
                        with_correlation
                    )
    write_observations_to_binary(observations, dataset_name)
    return observations
