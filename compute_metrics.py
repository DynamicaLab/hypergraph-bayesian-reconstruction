import os
import numpy as np
from warnings import warn

from generation.hypergraph_generation import load_binary_hypergraph
from generation.observations_generation import load_binary_observations
from metrics import compute_and_save_all_metrics, compute_and_save_tendency_metrics
from modelling.models import models, PES
from utils.config import ConfigurationParserWithModels, get_config, get_dataset_name
from utils.output import create_output_directories, get_tendency_sampling_directory, observations_filename, get_output_directory_for


class ParserWithOptionalTendency(ConfigurationParserWithModels):
    def __init__(self):
        super().__init__()
        group = self.parser.add_mutually_exclusive_group()
        group.add_argument('--mu1', action="store_true",
                            help="Vary mu1 and evaluate metric tendencies for given hypergraph")
        group.add_argument('--mu2', action="store_true",
                            help="Vary mu2 and evaluate metric tendencies for given hypergraph")

args = ParserWithOptionalTendency().parse_args()

config = get_config(args)
dataset_name = get_dataset_name(args)
inference_models = [models[model_name](config) for model_name in args.models]


create_output_directories(dataset_name, args.models)

hypergraph = load_binary_hypergraph(dataset_name)
if hypergraph is None and not args.o:
    warn("No hypergraph binary file found. The \"use groundtruth\" flag is automatically disabled. Run \"generate_data.py\" if the flag is desired.")



if args.mu1 or args.mu2:
    if args.mu1:
        varied_parameter_values = config["tendency", "varying mu1", "mu1"]
        parameters_values = [[config["tendency", "varying mu1", "mu0"], mu1, config["tendency", "varying mu1", "mu2"]] for mu1 in varied_parameter_values]
    elif args.mu2:
        varied_parameter_values = config["tendency", "varying mu2", "mu2"]
        parameters_values = [[config["tendency", "varying mu1", "mu0"], config["tendency", "varying mu2", "mu1"], mu2] for mu2 in varied_parameter_values]


    observation_id_to_do = np.arange(0, 10)

    for observation_parameters, varied_value in zip(parameters_values, varied_parameter_values):
        for model in inference_models:
            for observation_id in observation_id_to_do:
                print(f"Computing metrics of {model.complete_name} with means {observation_parameters}")

                sampling_directory = get_tendency_sampling_directory(args, dataset_name, model.name, varied_value, observation_id)
                observations = np.load(os.path.join(sampling_directory, observations_filename))

                swap_edge_types = observation_parameters[1]>observation_parameters[2] and model.name == PES.name
                compute_and_save_tendency_metrics(sampling_directory, observations, hypergraph, config["sampling", "sample size"],
                                                  model, config["metrics", "generated observations number"], swap_edge_types)

else:
    observations = load_binary_observations(dataset_name)
    if observations is None:
        raise RuntimeError("No observations found for given dataset. Run \"generate_data.py\" before sampling.")


    for model in inference_models:
        print("Computing metrics for model " + model.complete_name)
        sample_directory = get_output_directory_for("inference", dataset_name, model.name) + "/"
        swap_edge_types = not config["sampling", "mu1<mu2"] and model.name == PES.name
        compute_and_save_all_metrics(sample_directory, observations, hypergraph, config["sampling", "sample size"],
                                        model, config["metrics", "generated observations number"], swap_edge_types)
