import os, sys
import re
import numpy as np
from matplotlib import pyplot
from tqdm import tqdm

import plot_setup
sys.path.append("../")
from modeling.models import models
from modeling.config import ConfigurationParserWithModels, get_dataset_name, get_config
from modeling.output import get_output_directory_for, metrics_filename

import pygrit


def get_sample_number(file_name):
    expr = re.compile(r"hypergraph([0-9]+).*")
    match = expr.search(file_name)
    if match is not None:
        return int(match.group(1))
    else:
        return -1


def get_iteration_number(file_name):
    return int(file_name[-1])


# From https://stackoverflow.com/questions/643699/how-can-i-use-numpy-correlate-to-do-autocorrelation
def autocorrelation(sequence, lags):
    corr=[1. if ell==0 else np.corrcoef(sequence[ell:], sequence[:-ell])[0][1] for ell in lags]
    return np.array(corr)


os.chdir("../")


class DiagnosisParser(ConfigurationParserWithModels):
    def __init__(self):
        super().__init__()
        self.parser.add_argument('--gt', action="store_true",
                            help="Compare empirical to ground truth initialization. Not available with -o.")

args = DiagnosisParser().parser.parse_args()
dataset_name = get_dataset_name(args)
config = get_config(args)

if args.gt and args.o:
    raise ValueError("Cannot compare to ground truth with -o.")

inference_models = [models[model_name](config) for model_name in args.models]

for model in inference_models:
    sample_location = get_output_directory_for("diagnosis", dataset_name, model.name)
    sample_directories = os.listdir(sample_location)
    sample_directories.sort(key=get_iteration_number)

    if len(sample_directories) == 0:
        continue

    if model.name == "phg":
        distance = pygrit.get_global_hamming_distance
    else:
        distance = pygrit.get_edge_hamming_distance

    if args.gt:
        sample_directories = list(filter(lambda x: get_iteration_number(x)==8, sample_directories))
        fig, axes = pyplot.subplots(1, 1, figsize=(6, 4))

        zip_distances = []
        iterations = []
        sample_files = os.listdir(os.path.join(sample_location, sample_directories[0]))
        sample_files.sort(key=get_sample_number)

        for sample_file in tqdm(sample_files):
            if not sample_file.endswith("_triangles"):
                continue
            iterations.append(get_sample_number(sample_file))

            # This assumes that the same iterations were saved in both chains
            zip_distances.append(distance(
                pygrit.Hypergraph.load_from_binary(os.path.join(
                    sample_location, sample_directories[0], sample_file.removesuffix("_triangles"))
                ),
                pygrit.Hypergraph.load_from_binary(os.path.join(
                    sample_location, sample_directories[1], sample_file.removesuffix("_triangles"))
                )
            ))
        axes.plot(iterations, zip_distances)
        axes.set_ylabel("Hamming distance")
        axes.set_xlabel("Iteration")

    else:
        fig, axes = pyplot.subplots(4, len(sample_directories), figsize=(12, len(sample_directories)*2))

        for j, iteration_directory in tqdm(enumerate(sample_directories)):
            lagged_distance = []
            distance_to_first = [0]
            iterations = []
            first_sample, previous_sample = None, None

            sample_files = os.listdir(os.path.join(sample_location, iteration_directory))
            sample_files.sort(key=get_sample_number)

            for sample_element_file in tqdm(sample_files):
                if sample_element_file.endswith("_triangles"):
                    iterations.append(get_sample_number(sample_element_file))
                    sample = pygrit.Hypergraph.load_from_binary(os.path.join(sample_location, iteration_directory, sample_element_file.removesuffix("_triangles")))

                    if previous_sample is None:
                        first_sample = sample
                    else:
                        lagged_distance.append(distance(sample, previous_sample))
                        distance_to_first.append(distance(sample, first_sample))

                    previous_sample = sample

                elif "likelihood" in sample_element_file:
                    likelihood = np.fromfile(os.path.join(sample_location, iteration_directory, sample_element_file), dtype=np.double)
                elif "distances" in sample_element_file:
                    relative_distances = np.fromfile(os.path.join(sample_location, iteration_directory, sample_element_file), dtype=np.double)

            axes[0, j].plot(iterations, distance_to_first)
            axes[1, j].plot(iterations[1:], lagged_distance)
            axes[0, j].set_title(f"At Gibbs iteration {iteration_directory[-1]}")
            axes[-1, j].set_xlabel("Iteration")

            lags = np.arange(50)
            axes[2, j].bar(lags, np.abs(autocorrelation(likelihood, lags)))
            #axes[2, j].plot(iterations, likelihood)
            axes[3, j].plot(iterations, relative_distances)

        axes[0, 0].set_ylabel("Distance to\nfirst sample")
        axes[1, 0].set_ylabel("Distance to\nprevious sample")
        axes[2, 0].set_ylabel("Likelihood\nautocorrelation")
        axes[3, 0].set_ylabel("Relative\nlikelihood distance")

    fig.tight_layout()
    pyplot.show()
