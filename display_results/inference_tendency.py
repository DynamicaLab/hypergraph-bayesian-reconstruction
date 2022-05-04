import os, sys
import numpy as np
from matplotlib import pyplot
from numpy.lib.function_base import percentile
from scipy.special import gammainc, gammaincc
import warnings

import plot_setup
sys.path.append("../")
import modeling.metrics as metrics
from generation.hypergraph_generation import load_binary_hypergraph
from generation.observations_generation import load_binary_observations
from modeling.config import ConfigurationParserWithModels, get_json, get_dataset_name, get_config
from modeling.output import metrics_filename, get_tendency_sampling_directory, observations_filename
import pygrit

os.chdir("../")
warnings.formatwarning = lambda msg, *args, **kwargs: f'Warning: {msg}\n'


class TendencyParser(ConfigurationParserWithModels):
    def __init__(self):
        super().__init__()
        self.parser.add_argument
        group = self.parser.add_mutually_exclusive_group(required=True)
        group.add_argument('--mu1', action="store_true",
                            help="Vary mu1 and evaluate metric tendencies for given hypergraph")
        group.add_argument('--mu2', action="store_true",
                            help="Vary mu2 and evaluate metric tendencies for given hypergraph")


def compute_overlap(_p1, _p2, _mu1, _mu2):
    if _p1==0 or _p2==0:
        return 0

    if _mu1<_mu2:
        mu1, mu2 = _mu1, _mu2
        p1, p2 = _p1, _p2
    else:
        mu2, mu1 = _mu1, _mu2
        p2, p1 = _p1, _p2


    k_c = np.ceil( ( mu1-mu2 - np.log(p1)+np.log(p2) )/( np.log(mu1)-np.log(mu2) ) )
    if k_c<0:
        k_c = 0
    return 1/(p1+p2)*( p2*gammainc(mu2, k_c+1) + p1*gammaincc(mu1, k_c+1) )


args = TendencyParser().parser.parse_args()
observations_ids = np.arange(0, 10)

if args.o:
    raise ValueError("Observation datasets cannot be used to make this analysis.")

config = get_config(args)
dataset_name = get_dataset_name(args)


statistics = {}
metric_names = set()
percentiles = np.array([0.025, 0.25, 0.5, 0.75, 0.975])

if args.mu1:
    parameter_values = config["tendency", "varying mu1", "mu1"]
elif args.mu2:
    parameter_values = config["tendency", "varying mu2", "mu2"]
values_without_metrics = []

confusion_matrix_normalization = None
ground_truth_proportions = None

for model_name in args.models:
    statistics[model_name] = {}
    for parameter_value in parameter_values:
        aggregated_metrics = None

        for observation_id in observations_ids:
            sampling_directory = get_tendency_sampling_directory(args, dataset_name, model_name, parameter_value, observation_id)
            observations = np.load(os.path.join(sampling_directory, observations_filename))
            observations_sum = np.sum(observations)/2

            metric_filename = os.path.join(sampling_directory, metrics_filename)
            try:
                model_metrics = get_json( metric_filename )
            except FileNotFoundError:
                warnings.warn(f"Metrics for observation {observation_id} of parameter={parameter_value} not found for model {model_name}.")
                continue

            if aggregated_metrics is None:
                aggregated_metrics = { key: [] for key in list(model_metrics.keys())+["class entropy"] }
                metric_names.update(model_metrics.keys())
                metric_names.add("class entropy")

            for metric_name, metric_values in model_metrics.items():
                if metric_name != metrics.PairwiseObservationsAverage.name:
                    if metric_name == metrics.ConfusionMatrix.name:
                        corrected_metric = np.copy( np.array(metric_values[0]).reshape(3, 3) )
                        # swap columns 2 and 3 if multiplex model predicts no strong edge
                        if np.sum(corrected_metric[:, 2]) == 0 and model_name=="pes":
                            corrected_metric.T[[1, 2]] = corrected_metric.T[[2, 1]]

                        aggregated_metrics[metric_name].append(corrected_metric.flatten())
                        class_proportions = np.sum(corrected_metric, axis=0).astype(np.double)
                        class_proportions = class_proportions[class_proportions>0]
                        class_proportions /= np.sum(class_proportions)
                        aggregated_metrics["class entropy"].append(np.sum(-class_proportions*np.log(class_proportions)/np.log(3)))

                        if confusion_matrix_normalization is None:
                            edgetype_count = np.sum(corrected_metric, axis=1)
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
                statistics[model_name][metric_name] = { key: [] for key in ["mean", "std"]+["centile"+str(centile) for centile in percentiles] }
            metric_statistics = statistics[model_name][metric_name]

            if metric_name not in [metrics.ConfusionMatrix.name, metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:
                metric_values = np.array(metric_values).flatten()

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
    os.mkdir(figures_directory)

mu0 = config["tendency", "varying mu1", "mu0"]
if args.mu1:
    mu2 = config["tendency", "varying mu1", "mu2"]
    full_param_values = [(mu0, mu1, mu2) for mu1 in parameter_values]
elif args.mu2:
    mu1 = config["tendency", "varying mu2", "mu1"]
    full_param_values = [(mu0, mu1, mu2) for mu2 in parameter_values]


for metric_name in metric_names:
    if metric_name == metrics.PairwiseObservationsAverage.name:
        continue

    varied_parameter = "$\\mu_1$" if args.mu1 else "$\\mu_2$"

    if metric_name == metrics.ConfusionMatrix.name:
        fig, axes = pyplot.subplots(3, 3, figsize=(7, 4), sharex=True)
        for i, hyperedge_type in enumerate(["Hole", "Edge", "Triangle"]):
            axes[i, 0].set_ylabel(r"$\ell_{ij}="+str(i)+"$")
            axes[i, 0].yaxis.set_label_coords(-0.3, 0.5)
            axes[2, i].set_xlabel(varied_parameter)
            axes[0, i].set_title(r"$\hat\ell_{ij}="+str(i)+"$")
        fig2, axes2 = pyplot.subplots(3, 3, figsize=(7, 4), sharex=True)

    elif metric_name in [metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:
        fig, axes = pyplot.subplots(1, 3, figsize=(8, 3))
        for edgetype, ax in enumerate(axes):
            ax.set_xlabel(varied_parameter)
            ax.set_title("$\\ell_{ij}="+str(edgetype)+"$")
        axes[0].set_ylabel(metric_name)

    else:
        fig, axes = pyplot.subplots(1, figsize=(6, 4))
        axes.set_xlabel(varied_parameter)
        axes.set_ylabel(metric_name)


    for model_name in args.models:
        metric_statistics = statistics[model_name].get(metric_name)
        color = plot_setup.model_colors[model_name]

        if metric_name == metrics.ConfusionMatrix.name:
            for i, ax in enumerate(axes.flatten()):
                col, row = i%3, i//3

                get_element_stat = lambda stat: np.array(metric_statistics[stat])[:, i] / confusion_matrix_normalization[row, row]

                ax.plot(parameter_values, get_element_stat("centile0.5"), label=model_name, color=color, marker=".", clip_on=False)
                ax.fill_between(parameter_values, get_element_stat("centile0.025"), get_element_stat("centile0.975"), alpha=.1, color=color)
                ax.fill_between(parameter_values, get_element_stat("centile0.25"),  get_element_stat("centile0.75"), alpha=.2, color=color)
                ax.set_ylim(-0.05, 1.05)

                if col >= row:
                    transposed_index = col*3 + row
                    if col==row:
                        get_element_stat_sum = get_element_stat
                    else:
                        get_element_stat_sum = lambda stat: (np.array(metric_statistics[stat])[:, i]+np.array(metric_statistics[stat])[:, transposed_index]) \
                                                                / confusion_matrix_normalization[row, col]

                    axes2[row, col].plot(parameter_values, get_element_stat_sum("centile0.5"), label=model_name, color=color, marker=".", clip_on=False)
                    axes2[row, col].fill_between(parameter_values, get_element_stat_sum("centile0.025"), get_element_stat_sum("centile0.975"), alpha=.1, color=color)
                    axes2[row, col].fill_between(parameter_values, get_element_stat_sum("centile0.25"),  get_element_stat_sum("centile0.75"), alpha=.2, color=color)
                    axes2[row, col].set_ylim(-.05, 1.05)
                    if col != row:
                        axes2[row, col].plot(parameter_values, [compute_overlap(confusion_matrix_proportions[row], confusion_matrix_proportions[col], mu[row], mu[col]) for mu in full_param_values],
                                color="k")
                    else:
                        axes2[row, col].plot(parameter_values, [1-sum([compute_overlap(confusion_matrix_proportions[row], confusion_matrix_proportions[i], mu[row], mu[i]) for i in range(3) if i!=row])
                                                            for mu in full_param_values],
                                color="k")


        elif metric_name in [metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:
            for edgetype, ax in enumerate(axes):
                get_element_stat = lambda stat: np.array(metric_statistics[stat])[:, edgetype]

                ax.plot(parameter_values, get_element_stat("centile0.5"), label=model_name, color=color, marker=".")
                ax.fill_between(parameter_values, get_element_stat("centile0.025"), get_element_stat("centile0.975"), alpha=.1, color=color)
                ax.fill_between(parameter_values, get_element_stat("centile0.25"), get_element_stat("centile0.75"), alpha=.2, color=color)

        else:
            axes.plot(parameter_values, metric_statistics["centile0.5"], label=model_name, color=color, marker=".")
            axes.fill_between(parameter_values, metric_statistics["centile0.025"], metric_statistics["centile0.975"], alpha=.1, color=color)
            axes.fill_between(parameter_values, metric_statistics["centile0.25"], metric_statistics["centile0.75"], alpha=.2, color=color)


    pyplot.legend()
    fig.tight_layout()
    fig.subplots_adjust(hspace=.185, wspace=.285)
    fig.savefig(figures_directory+f"/{metric_name}_{'mu1' if args.mu1 else 'mu2'}.pdf", bbox_inches='tight')
    pyplot.close()
