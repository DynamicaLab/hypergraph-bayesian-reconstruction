from utils.config import get_config, get_dataset_name, GenericConfigurationParser
from utils.output import create_output_directories, erase_inference_diagnosis_results
from generation.hypergraph_generation import generate_and_write_hypergraph, transfer_hypergraph_data_to_binary
from generation.observations_generation import generate_and_write_observations, transfer_observation_data_to_binary


args = GenericConfigurationParser().parse_args()
config = get_config(args)
dataset_name = get_dataset_name(args)


create_output_directories(dataset_name)
if args.s:
    print("New creating random hypergraph")
    hypergraph = generate_and_write_hypergraph(config, dataset_name)

elif args.g:
    print("Transforming existing hypergraph into binary")
    hypergraph = transfer_hypergraph_data_to_binary(config, dataset_name)

if args.s or args.g:
    if hypergraph.get_size() != config["vertex number"]:
        print(f"Warning: Generated hypergraph has size {hypergraph.get_size()} while configuration file indicates"
                f" {config['vertex number']} vertices.")


if args.s or args.g:
    print("Creating new random observations")
    generate_and_write_observations(hypergraph, config, dataset_name, with_correlation=True)  # Analysed graphs are always with correlation
    print("Removing current inference and diagnosis results")
    erase_inference_diagnosis_results(dataset_name)
else:
    print("Transforming existing dataset into binary")
    transfer_observation_data_to_binary(config, dataset_name)
