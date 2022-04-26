import os
import re
import json
import warnings
import numpy as np
import shutil
import subprocess

import pygrit


main_output_directory = "raw_data/"
metrics_filename = "metrics.json"
hypergraph_filename = "hypergraph.bin"
observations_filename = "observations.npy"
diagnosis_iteration_prefix = "iteration"
chain_directory_prefix = "chain"

parameters_format = "parameters{}_{}.bin"
hypergraph_format = "hypergraph{}_{}.bin"
occurences_format = "occurences{}_edgetype{}.bin"
chain_directory_format = chain_directory_prefix+"{}"

chain_regex = re.compile(".*"+chain_directory_format.format(r"(\d+)$"))


def create_output_directories(dataset_name, model_names=None):
    if model_names is None:
        directory_path = get_output_directory_for("data", dataset_name)
        if not os.path.isdir(directory_path):
            os.makedirs(directory_path)
    else:
        for model_name in model_names:
            for output_type in ["diagnosis", "inference"]:
                directory_path = get_output_directory_for(output_type, dataset_name, model_name)
                if not os.path.isdir(directory_path):
                    os.makedirs(directory_path)

            for setup in ["varying_mu1", "varying_mu2"]:
                directory_path = os.path.join(get_output_directory_for("tendency", dataset_name, model_name), setup)
                if not os.path.isdir(directory_path):
                    os.makedirs(directory_path)


def prepare_diagnosis_directories(dataset_name, model_names, iterations):
    for model_name in model_names:
        directory_path = get_output_directory_for("diagnosis", dataset_name, model_name)
        for old_sample_directory in os.listdir(directory_path):
            shutil.rmtree(os.path.join(directory_path, old_sample_directory))

        for iteration in iterations:
            os.makedirs(os.path.join(directory_path, diagnosis_iteration_prefix)+str(iteration))


def remove_current_sample_tarballs(sample_directory):
    for filename in os.listdir(sample_directory):
        if filename.endswith(".tar.gz"):
            os.remove(os.path.join(sample_directory, filename))


def make_sample_a_tarball(sample_directory):
    if shutil.which("tar") is None:
        warnings.warn(f"Couldn't create a tar archive for directory \"{sample_directory}\": tar program not found.")
    else:
        for chain_directory in os.listdir(sample_directory):
            chain_number = None
            if chain_regex.match(chain_directory):
                full_path = os.path.join(sample_directory, chain_directory)
                subprocess.call(["tar", "-czf", full_path+".tar.gz", "-C", sample_directory, chain_directory, "--remove-files"])


def erase_sample(sample_directory):
    for directory in os.listdir(sample_directory):
        if os.path.isdir(directory) and chain_regex.match(directory):
            shutil.rmtree(os.path.join(sample_directory, directory))


def erase_inference_diagnosis_results(dataset_name):
    dataset_output_dir = get_output_directory_for("data", dataset_name)

    for model_name in os.listdir(dataset_output_dir):

        if os.path.isdir(os.path.join(dataset_output_dir, model_name)):
            for output_type in ["diagnosis", "inference"]:

                output_dir = get_output_directory_for(output_type, dataset_name, model_name)
                if os.path.isdir(output_dir):
                    shutil.rmtree(output_dir)


def remove_all_chains_but(chain_to_keep, sample_directory):
    for chain_directory in os.listdir(sample_directory):
        chain_number = None
        if chain_regex.match(chain_directory):
            chain_number = int(chain_regex.search(chain_directory).group(1))

            if chain_number != chain_to_keep:
                shutil.rmtree(os.path.join(sample_directory, chain_directory))


def get_output_directory_for(output_type, dataset_name, model_name=None):
    path = os.path.join(main_output_directory, dataset_name)

    if output_type == "data":
        pass
    elif model_name is None:
        raise ValueError("Invalid argument \"model_name\" for \"get_output_directory_for\". No model name was given.")
    elif output_type in ["diagnosis", "inference", "tendency"]:
        path = os.path.join(path, model_name, output_type)
    else:
        raise ValueError(f"Unknown output type {output_type}")

    return path


def get_tendency_sampling_directory(program_args, dataset_name, model_name, parameter_value, observation_id):
    setup_name = "varying_mu1" if program_args.mu1 else "varying_mu2"
    varied_parameter = "mu1" if program_args.mu1 else "mu2"

    return os.path.join(get_output_directory_for("tendency", dataset_name, model_name),
                        setup_name, varied_parameter+"="+str(parameter_value), "repetition"+str(observation_id))


def get_edgetype_probabilities_of_chain(chain, sample_directory, sample_size):
    chain_directory = os.path.join(sample_directory, chain_directory_format.format(chain))

    edgetype1_path = os.path.join(chain_directory, occurences_format.format(chain, 1))
    edgetype2_path = os.path.join(chain_directory, occurences_format.format(chain, 2))

    if os.path.isfile(edgetype1_path) and os.path.isfile(edgetype2_path):
        edgetype1_occurences = np.fromfile(edgetype1_path, dtype=np.uint64)
        edgetype2_occurences = np.fromfile(edgetype2_path, dtype=np.uint64)
        edgetype0_occurences = np.full_like(edgetype1_occurences, sample_size) - edgetype1_occurences - edgetype2_occurences

        return edgetype0_occurences/sample_size, edgetype1_occurences/sample_size, edgetype2_occurences/sample_size


def get_map_estimator(sample_directory, sample_size, model, observations):
    best_sample = None
    best_loglikelihood = None

    for chain in find_chains(sample_directory):
        *chain_best_sample, chain_best_loglikelihood = get_map_estimator_of_chain(chain, sample_directory, sample_size, model, observations)

        if best_loglikelihood is None or chain_best_loglikelihood > best_loglikelihood:
            best_sample = chain_best_sample
    return best_sample


def get_map_estimator_of_chain(chain, sample_directory, sample_size, model, observations):
    best_sample = None
    best_loglikelihood = None

    for hypergraph_path, parameters_path in get_sample_files(sample_directory, sample_size):
        hypergraph = pygrit.Hypergraph.load_from_binary(hypergraph_path)
        parameters = np.fromfile(parameters_path, dtype=np.double)
        loglikelihood = model.get_loglikelihood(hypergraph, parameters, observations)

        if best_loglikelihood is None or loglikelihood > best_loglikelihood:
            best_sample = (hypergraph, parameters)

    if best_sample is None:
        raise RuntimeError(f"Couldn't find the MAP estimator for chain {chain} of \"{sample_directory}\".")

    return *best_sample, best_loglikelihood


def get_sample(sample_directory, sample_size):
    for hypergraph_path, parameters_path in get_sample_files(sample_directory, sample_size):
        yield pygrit.Hypergraph.load_from_binary(hypergraph_path), np.fromfile(parameters_path, dtype=np.double)

def get_sample_files(sample_directory, sample_size):
    for chain in find_chains(sample_directory):
        for sample_element in  get_sample_files_of_chain(chain, sample_directory, sample_size):
            yield sample_element

def get_sample_of_chain(chain, sample_directory, sample_size):
    for hypergraph_path, parameters_path in get_sample_files_of_chain(chain, sample_directory, sample_size):
        yield pygrit.Hypergraph.load_from_binary(hypergraph_path), np.fromfile(parameters_path, dtype=np.double)

def get_sample_files_of_chain(chain, sample_directory, sample_size):
    chain_directory = os.path.join(sample_directory, chain_directory_format.format(chain))
    for i in range(sample_size):
        hypergraph_path = os.path.join(chain_directory, hypergraph_format.format(chain, i))
        parameters_path = os.path.join(chain_directory, parameters_format.format(chain, i))

        if os.path.isfile(hypergraph_path+"_edges") and os.path.isfile(hypergraph_path+"_triangles")\
                and os.path.isfile(parameters_path):
            yield hypergraph_path, parameters_path


def find_chains(sample_directory):
    chains = []

    for directory in os.listdir(sample_directory):
        if os.path.isdir(os.path.join(sample_directory, directory)) and chain_regex.match(directory):
            chains.append( int(chain_regex.search(directory).group(1)) )
    return chains


class NpEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.integer):
            return int(obj)
        if isinstance(obj, np.floating):
            return float(obj)
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        return super(NpEncoder, self).default(obj)


def write_metrics(sample_directory, metrics):
    filename = os.path.join(sample_directory, metrics_filename)
    with open(filename, "w") as file_stream:
        json.dump(metrics, file_stream, cls=NpEncoder)
