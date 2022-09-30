import os, sys
import numpy as np
import json
from tqdm import tqdm
from numpy.lib.function_base import percentile
import warnings

import plot_setup

warnings.formatwarning = lambda msg, *args, **kwargs: f'Warning: {msg}\n'
sys.path.append("../")

import modeling.metrics as metrics
from modeling.config import ConfigurationParserWithModels, get_json, get_dataset_name, get_config
from modeling.output import metrics_filename, get_tendency_sampling_directory, observations_filename

os.chdir("../")


class TendencyParser(ConfigurationParserWithModels):
    def __init__(self):
        super().__init__()
        self.parser.add_argument
        group = self.parser.add_mutually_exclusive_group(required=True)
        group.add_argument('--mu1', action="store_true",
                            help="Vary mu1 and evaluate metric tendencies for given hypergraph")
        group.add_argument('--mu2', action="store_true",
                            help="Vary mu2 and evaluate metric tendencies for given hypergraph")


args = TendencyParser().parser.parse_args()
observations_ids = np.arange(0, 200)

if args.o:
    raise ValueError("Observation datasets cannot be used to make this analysis.")

config = get_config(args)
dataset_name = get_dataset_name(args)

statistics = {}
percentiles = np.array([0.025, 0.25, 0.5, 0.75, 0.975])

if args.mu1:
    parameter_values = config["tendency", "varying mu1", "mu1"]
elif args.mu2:
    parameter_values = config["tendency", "varying mu2", "mu2"]
values_without_metrics = []

confusion_matrix_normalization = None
ground_truth_proportions = None

for model_name in args.models:
    print("Computing metrics of model", model_name)
    statistics[model_name] = {}
    for parameter_value in tqdm(parameter_values):
        aggregated_metrics = None

        for observation_id in observations_ids:
            sampling_directory = get_tendency_sampling_directory(args, dataset_name,
                                            model_name, parameter_value, observation_id)
            observations = np.load(os.path.join(sampling_directory, observations_filename))
            observations_sum = np.sum(observations)/2

            metric_filename = os.path.join(sampling_directory, metrics_filename)
            try:
                model_metrics = get_json( metric_filename )
            except FileNotFoundError:
                warnings.warn(f"Metrics for observation {observation_id} of "
                        f"parameter={parameter_value} not found for model {model_name}.")
                continue

            if aggregated_metrics is None:
                aggregated_metrics = {
                    key: [] for key in
                        list(model_metrics.keys())+["class entropy", "confusion summary"]
                }

            for metric_name, metric_values in model_metrics.items():
                if metric_name == metrics.PairwiseObservationsAverage.name:
                    continue

                if metric_name == metrics.ConfusionMatrix.name:
                    C = np.copy( np.array(metric_values[0]).reshape(3, 3) )
                    # swap columns 2 and 3 if multiplex model predicts no strong edge
                    if np.sum(C[:, 2]) == 0 and model_name=="pes" and args.mu1:
                        C.T[[1, 2]] = C.T[[2, 1]]

                    aggregated_metrics[metric_name].append(C.ravel())
                    class_proportions = np.sum(C, axis=0).astype(np.double)
                    class_proportions = class_proportions[class_proportions>0]
                    class_proportions /= np.sum(class_proportions)
                    aggregated_metrics["class entropy"].append(
                            np.sum(-class_proportions*np.log(class_proportions)/np.log(3))
                        )

                    pairwise_interaction_number = np.sum(C[1:])
                    aggregated_metrics["confusion summary"].append(
                            (pairwise_interaction_number-C[1, 1]-C[2, 2])/pairwise_interaction_number
                        )

                    if confusion_matrix_normalization is None:
                        edgetype_count = np.sum(C, axis=1)
                        confusion_matrix_proportions = edgetype_count/np.sum(edgetype_count)
                        confusion_matrix_normalization = np.diag(edgetype_count)
                        for i, j in [(0, 1), (0, 2), (1, 2)]:
                            confusion_matrix_normalization[i, j] = edgetype_count[i] + edgetype_count[j]
                            confusion_matrix_normalization[j, i] = edgetype_count[i] + edgetype_count[j]
                        confusion_matrix_normalization = confusion_matrix_normalization

                elif metric_name == metrics.SumAbsoluteResiduals.name:
                    aggregated_metrics[metric_name].append(metric_values/observations_sum)
                else:
                    aggregated_metrics[metric_name].append(metric_values)

        if aggregated_metrics is None:
            values_without_metrics.append(parameter_value)
            continue

        for metric_name, metric_values in aggregated_metrics.items():
            if metric_name == metrics.PairwiseObservationsAverage.name:
                continue

            if statistics[model_name].get(metric_name) is None:
                statistics[model_name][metric_name] = {
                        key: [] for key in
                            ["mean", "std"]+["centile"+str(centile) for centile in percentiles]
                        }
            metric_statistics = statistics[model_name][metric_name]

            if metric_name not in [metrics.ConfusionMatrix.name, metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:
                metric_values = np.array(metric_values).ravel()

            if metric_name in [metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:
                axis=(0, 2)
            else:
                axis=0

            metric_statistics["mean"].append( np.mean(metric_values, axis=axis) )
            metric_statistics["std"].append( np.std(metric_values, axis=axis) )

            for centile, val in zip(percentiles, np.percentile(metric_values, percentiles*100, axis=axis)):
                metric_statistics["centile"+str(centile)].append( val )

for parameter_value in parameter_values.copy():
    if values_without_metrics.count(parameter_value) == len(args.models):
        parameter_values.remove(parameter_value)


figures_directory = plot_setup.get_figure_dir(dataset_name)
if not os.path.isdir(figures_directory):
    os.makedirs(figures_directory)

filename = os.path.join(
        figures_directory,
        f"tendency_data_mu{'1' if args.mu1 else '2'}.json"
    )


class NpEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.integer):
            return int(obj)
        if isinstance(obj, np.floating):
            return float(obj)
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        return super(NpEncoder, self).default(obj)


with open(filename, 'w') as file_stream:
    plot_data = {
            "metrics": statistics.copy(),
            "confusion normalization": confusion_matrix_normalization
        }
    json.dump(plot_data, file_stream, cls=NpEncoder)
