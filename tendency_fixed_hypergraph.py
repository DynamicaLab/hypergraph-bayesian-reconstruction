import os
import shutil
import numpy as np
from mpi4py import MPI

from generation.hypergraph_generation import load_binary_hypergraph
from generation.observations_generation import generate_observations
from modeling.metrics import compute_and_save_tendency_metrics
from modeling.models import models, PES, PHG
from modeling.config import ConfigurationParserWithModels, get_config, get_dataset_name
from modeling.output import create_output_directories, get_tendency_sampling_directory, observations_filename,\
                            make_sample_a_tarball, remove_current_sample_tarballs


class TendencyParser(ConfigurationParserWithModels):
    def __init__(self):
        super().__init__()
        group = self.parser.add_mutually_exclusive_group(required=True)
        group.add_argument('--mu1', action="store_true",
                            help="Vary mu1 and evaluate metric tendencies for given hypergraph.")
        group.add_argument('--mu2', action="store_true",
                            help="Vary mu2 and evaluate metric tendencies for given hypergraph.")
        group.add_argument('--rates', action="store_true",
                            help="Vary both mu1 and mu2 according to the same rate r (e.g. mu1=2r, mu2=3r).")


def thread_has_responsability(task_number, rank):
    global world_size
    return (task_number % world_size) == rank


observation_id_to_do = np.arange(0, 200)


if __name__ == "__main__":
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    world_size = comm.Get_size()


    args = TendencyParser().parse_args()

    config = get_config(args)
    dataset_name = get_dataset_name(args)
    inference_models = [models[model_name](config) for model_name in args.models]


    if rank == 0:
        create_output_directories(dataset_name, args.models)

    hypergraph = load_binary_hypergraph(dataset_name)
    if hypergraph is None and not args.o:
        raise RuntimeError("No hypergraph binary file found. Run \"generate_data.py\" before sampling.")


    if args.mu1:
        varied_parameter_values = config["tendency", "varying mu1", "mu1"]
        parameters_values = [[config["tendency", "varying mu1", "mu0"], mu1, config["tendency", "varying mu1", "mu2"]]
                             for mu1 in varied_parameter_values]
    elif args.mu2:
        varied_parameter_values = config["tendency", "varying mu2", "mu2"]
        parameters_values = [[config["tendency", "varying mu1", "mu0"], config["tendency", "varying mu2", "mu1"], mu2]
                             for mu2 in varied_parameter_values]
    elif args.rates:
        mu0 = config["tendency", "varying rates", "mu0"]
        base_mu1 = config["tendency", "varying rates", "base mu1"]
        base_mu2 = config["tendency", "varying rates", "base mu2"]

        varied_parameter_values = config["tendency", "varying rates", "lambda"]
        parameters_values = [[mu0, base_mu1*lambda_, base_mu2*lambda_]
                             for lambda_ in varied_parameter_values]
    else:
        raise ValueError("Unknown tendency experiment type.")

    # Generate observations
    task_id = -1
    for observation_parameters, varied_value in zip(parameters_values, varied_parameter_values):
        # Task are not distributed inner most loop to avoid race conditions when creating directories
        task_id += 1
        if not thread_has_responsability(task_id, rank):
            continue

        for observation_id in observation_id_to_do:
            observations = generate_observations(hypergraph, config["synthetic generation", "observation process"], observation_parameters, True)

            for model in inference_models:
                sampling_directory = get_tendency_sampling_directory(args, dataset_name, model.name, varied_value, observation_id)

                if not os.path.isdir(sampling_directory):
                    os.makedirs(sampling_directory)
                np.save(os.path.join(sampling_directory, observations_filename), observations)

    comm.Barrier()


    # Sample
    task_id = -1
    for observation_parameters, varied_value in zip(parameters_values, varied_parameter_values):
        for model in inference_models:
            if args.rates:
                # Adjust prior average of mu1 and mu2.
                model_hyperparameters = config["models", model.name, "hyperparameters"]
                alpha1 = model_hyperparameters[6]
                alpha2 = model_hyperparameters[8]
                model_hyperparameters[7] = alpha1/observation_parameters[1]
                model_hyperparameters[9] = alpha2/observation_parameters[2]
                model.sampler.set_hyperparameters(model_hyperparameters)

            for observation_id in observation_id_to_do:
                task_id += 1
                if not thread_has_responsability(task_id, rank):
                    continue
                print(f"Sampling {model.complete_name} with means {observation_parameters} "
                        f"{observation_id+1}/{len(observation_id_to_do)}")

                sampling_directory = get_tendency_sampling_directory(args, dataset_name, model.name, varied_value, observation_id)
                observations = np.load(os.path.join(sampling_directory, observations_filename))

                groundtruth = (hypergraph,
                        [None]*2+observation_parameters if model.name == PHG.name\
                        else [None]*2+[observation_parameters[0], min(observation_parameters[1:]), max(observation_parameters[1:])]
                        )

                model.sample(observations, ground_truth=groundtruth, sampling_directory=sampling_directory,
                             mu1_smaller_mu2=observation_parameters[1]<observation_parameters[2], verbose=0)

                swap_edge_types = observation_parameters[1]>observation_parameters[2] and model.name == PES.name
                compute_and_save_tendency_metrics(sampling_directory, observations, hypergraph, config["sampling", "sample size"],
                                                  model, config["metrics", "generated observations number"], swap_edge_types)

                # Useful for computers where number of files is restricted
                remove_current_sample_tarballs(sampling_directory)
                make_sample_a_tarball(sampling_directory)
