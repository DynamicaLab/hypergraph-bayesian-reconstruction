import os
import shutil
import numpy as np

from generation.hypergraph_generation import load_binary_hypergraph
from generation.observations_generation import generate_observations
from modeling.metrics import ConfusionMatrix, compute_metrics
from modeling.models import models, PES, PHG
from modeling.config import ConfigurationParserWithModels, get_config, get_dataset_name
from modeling.output import create_output_directories, get_output_directory_for, write_metrics


if __name__ == "__main__":
    observation_parameters = [0.01, 40, 50]

    args = ConfigurationParserWithModels().parse_args()

    config = get_config(args)
    dataset_name = get_dataset_name(args)
    inference_models = [models[model_name](config) for model_name in args.models]

    create_output_directories(dataset_name, args.models)

    hypergraph = load_binary_hypergraph(dataset_name)
    if hypergraph is None and not args.o:
        raise RuntimeError("No hypergraph binary file found. Run \"generate_data.py\" before sampling.")


    output_directory = os.path.join(get_output_directory_for("data", dataset_name))
    sampling_directory = os.path.join(output_directory, "tmp")
    if not os.path.isdir(sampling_directory):
        os.mkdir(sampling_directory)

    confusionMatrices = {model.name: [] for model in inference_models}
    repetitions = 10
    for i in range(repetitions):
        observations = generate_observations(hypergraph, config["synthetic generation", "observation process"],
                                                observation_parameters, True)

        for model in inference_models:
            print(f"{model.complete_name} {i+1}/{repetitions}")
            groundtruth = (hypergraph,
                    [None]*2+observation_parameters if model.name == PHG.name\
                    else [None]*2+[observation_parameters[0], min(observation_parameters[1:]), max(observation_parameters[1:])]
                    )

            model.sample(observations, ground_truth=groundtruth, sampling_directory=sampling_directory,
                         mu1_smaller_mu2=observation_parameters[1]<observation_parameters[2], verbose=0)

            swap_edge_types = observation_parameters[1]>observation_parameters[2] and model.name == PES.name

            metrics = compute_metrics(sampling_directory, config["sampling", "sample size"], model,
                        config["metrics", "generated observations number"],
                        { "sample_metrics": [ ConfusionMatrix(hypergraph, swap_edge_types) ],
                          "sample_point_metrics": [], "posterior_predictive_metrics": [] }
                    )
            confusionMatrices[model.name].append(metrics[ConfusionMatrix.name])

    write_metrics(output_directory, confusionMatrices)
    shutil.rmtree(sampling_directory)
