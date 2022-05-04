from warnings import warn

from generation.hypergraph_generation import load_binary_hypergraph
from generation.observations_generation import load_binary_observations
from modeling.metrics import compute_and_save_all_metrics
from modeling.models import models, PES, PHG
from modeling.config import ConfigurationParserWithModels, get_config, get_dataset_name
from modeling.output import create_output_directories, get_output_directory_for


args = ConfigurationParserWithModels().parse_args()

config = get_config(args)
dataset_name = get_dataset_name(args)
inference_models = [models[model_name](config) for model_name in args.models]


create_output_directories(dataset_name, args.models)

observations = load_binary_observations(dataset_name)
if observations is None:
    raise RuntimeError("No observations found for given dataset. Run \"generate_data.py\" before sampling.")

hypergraph = load_binary_hypergraph(dataset_name)
if hypergraph is None and not args.o:
    warn("No hypergraph binary file found. The \"use groundtruth\" flag is automatically disabled. Run \"generate_data.py\" if the flag is desired.")


for model in inference_models:
    print("Processing model " + model.complete_name)
    sample_directory = get_output_directory_for("inference", dataset_name, model.name) + "/"
    mu1_smaller_mu2 = config["sampling", "mu1<mu2"]

    if args.s or args.g:
        observation_parameters = config["synthetic generation", "observation parameters"]
        known_parameters = [None]*2+observation_parameters if model.name == PHG.name\
                           else [None]*2+[observation_parameters[0], min(observation_parameters[1:]), max(observation_parameters[1:])]
    else:
        known_parameters = [None]*5

    model.sample(observations, ground_truth=(hypergraph, known_parameters), sampling_directory=sample_directory, mu1_smaller_mu2=mu1_smaller_mu2)

    print("Computing metrics")
    swap_edge_types = not mu1_smaller_mu2 and model.name == PES.name
    compute_and_save_all_metrics(sample_directory, observations, hypergraph, config["sampling", "sample size"],
                                    model, config["metrics", "generated observations number"], swap_edge_types)
    print()
