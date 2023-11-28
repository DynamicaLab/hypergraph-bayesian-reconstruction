from warnings import warn
import os

from generation.hypergraph_generation import load_binary_hypergraph
from generation.observations_generation import load_binary_observations
from modeling.models import models, PHG
from modeling.config import ConfigurationParserWithModels, get_config, get_dataset_name
from modeling.output import create_output_directories, get_output_directory_for, prepare_diagnosis_directories, diagnosis_iteration_prefix


class DiagnosticParser(ConfigurationParserWithModels):
    def __init__(self):
        super().__init__()
        self.parser.add_argument("-p", type=int, dest="points", default="100")
        self.parser.add_argument("-i", type=int, dest="iterations", nargs="+", default=["0", "1"])

args = DiagnosticParser().parse_args()

config = get_config(args)
dataset_name = get_dataset_name(args)
inference_models = [models[model_name](config) for model_name in args.models]


create_output_directories(dataset_name, args.models)
prepare_diagnosis_directories(dataset_name, args.models, args.iterations)

observations = load_binary_observations(dataset_name)
if observations is None:
    raise RuntimeError("No observations found for given dataset. Run \"generate_data.py\" before sampling.")

hypergraph = load_binary_hypergraph(dataset_name)
if hypergraph is None and not args.o:
    warn("No hypergraph binary file found. The \"use groundtruth\" flag is automatically disabled. Run \"generate_data.py\" if the flag is desired.")


for model in inference_models:
    print("Processing model " + model.complete_name)
    sample_directory = os.path.join(
            get_output_directory_for("diagnosis", dataset_name, model.name),
            diagnosis_iteration_prefix
        )

    if args.s or args.g:
        observation_parameters = config["synthetic generation", "observation parameters"]
        known_parameters = [None]*2+observation_parameters if model.name == PHG.name\
                           else [None]*2+[observation_parameters[0], min(observation_parameters[1:]), max(observation_parameters[1:])]
    else:
        known_parameters = [None]*5

    model.sample_hypergraph_chain(
            observations, ground_truth=None,
            sampling_directory=sample_directory,
            mu1_smaller_mu2=config["sampling", "mu1<mu2"],
            use_ground_truth=False, iterations=args.iterations, points=args.points
    )
    if not args.o:
        model.sample_hypergraph_chain(
                observations, ground_truth=(hypergraph, known_parameters),
                sampling_directory=sample_directory+"gt", mu1_smaller_mu2=config["sampling", "mu1<mu2"],
                use_ground_truth=True, iterations=args.iterations, points=args.points
        )
    print()
