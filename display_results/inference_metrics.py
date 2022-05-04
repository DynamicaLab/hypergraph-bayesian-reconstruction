import os, sys
import numpy as np
from matplotlib import pyplot, colors
from numpy.lib.function_base import percentile

import plot_setup
sys.path.append("../")
import metrics
from utils.config import ConfigurationParserWithModels, get_json, get_dataset_name, get_config
from utils.output import get_output_directory_for, metrics_filename
from modeling.models import models
from generation.observations_generation import load_binary_observations
from generation.hypergraph_generation import load_binary_hypergraph
import pygrit

os.chdir("../")


def get_boxplot_stats(statistics, **kwargs):
    stats = {
            "mean"  : statistics.get("mean"),
            "med"   : statistics.get("centile0.5"),
            "q1"    : statistics.get("centile0.25"),
            "q3"    : statistics.get("centile0.75"),
            "whislo": statistics.get("centile0.025"),
            "whishi": statistics.get("centile0.975")
            }
    stats.update(kwargs)
    return stats

def map_dict_onto_array(dict, array):
    u, inv = np.unique(array, return_inverse = True)
    return np.array([dict[x] for x in u])[inv]


args = ConfigurationParserWithModels().parser.parse_args()
config = get_config(args)
inference_models = [models[model_name](config) for model_name in args.models]

dataset_name = get_dataset_name(args)
observations = load_binary_observations(dataset_name)
groundtruth_hypergraph = load_binary_hypergraph(dataset_name)


statistics = {}
metric_names = set()
model_number = len(args.models)

for model_name in args.models:
    model_metrics = get_json( os.path.join(get_output_directory_for("inference", dataset_name, model_name), metrics_filename) )
    statistics[model_name] = {}

    metric_names.update(model_metrics.keys())
    for metric_name, metric_values in model_metrics.items():
        if args.o and metric_name == metrics.ConfusionMatrix.name or not metric_values:
            continue
        statistics[model_name][metric_name] = {}

        if metric_name in [metrics.DiscrepancyPValue.name, metrics.ConfusionMatrix.name, metrics.WAIC.name]:
            statistics[model_name][metric_name]["mean"] = metric_values

        elif metric_name == metrics.PairwiseObservationsAverage.name:
            metric_values = np.array(metric_values[0])
            statistics[model_name][metric_name]["mean"] = metric_values[:, 0]
            statistics[model_name][metric_name]["std"]  = np.sqrt(metric_values[:, 1])

        elif metric_name in [metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:
            if groundtruth_hypergraph is None:
                metric_values = [metric_values]

            statistics[model_name][metric_name] = []

            for hyperedge_metric in metric_values:
                hyperedge_statistics = {}
                hyperedge_statistics["mean"] = np.mean(hyperedge_metric, axis=0)

                percentiles = [0.025, 0.25, 0.5, 0.75, 0.975]
                for centile, val in zip(percentiles, np.percentile(hyperedge_metric, percentiles, axis=0)):
                    hyperedge_statistics["centile"+str(centile)] = val

                statistics[model_name][metric_name].append(hyperedge_statistics)


figures_directory = plot_setup.get_figure_dir(dataset_name)
if not os.path.isdir(figures_directory):
    os.makedirs(figures_directory)


for metric_name in metric_names:
    fig = None

    if metric_name == metrics.ConfusionMatrix.name:
        if groundtruth_hypergraph is None:
            continue

        fig, axes = pyplot.subplots(1, len(args.models), figsize=(4*len(args.models), 4), sharex=True)
        if len(args.models) == 1:
            axes = [axes]

        for ax in axes:
            ax.set_ylabel("$\\ell_{ij}$", labelpad=10)
            ax.set_xlabel("$\\hat \\ell_{ij}$")

    elif metric_name == metrics.PairwiseObservationsAverage.name:
        if groundtruth_hypergraph is None:
            fig, axes = pyplot.subplots(1, 1, figsize=(6, 4))
            axes = [axes]
        else:
            fig, axes = pyplot.subplots(1, 3, figsize=(12, 4))

        axes[0].set_ylabel(metric_name)


    elif metric_name in [metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:
        if groundtruth_hypergraph is None:
            fig, axes = pyplot.subplots(1, figsize=(6, 4))
            boxplot_stats = []
            axes = [axes]
        else:
            fig, axes = pyplot.subplots(1, 3, figsize=(12, 4))
            boxplot_stats = [list() for i in range(3)]
            for edge_type, ax in enumerate(axes):
                ax.set_title("$\\ell_{ij}="+str(edge_type)+"$")

        axes[0].set_ylabel(metric_name)


    for i, model in enumerate(inference_models):
        metric_statistics = statistics[model.name].get(metric_name)
        if metric_statistics is None:
            continue

        color = plot_setup.model_colors[model.name]

        if metric_name in [metrics.DiscrepancyPValue.name, metrics.WAIC.name]:
            print(metric_name, model.complete_name, ":",  metric_statistics["mean"])

        elif metric_name == metrics.ConfusionMatrix.name:

            confusion_matrix = np.array(metric_statistics["mean"]).reshape(3, 3)
            cmap = colors.LinearSegmentedColormap.from_list("model_cmap", [color+"0A", color+"FF"])
            axes[i].imshow(confusion_matrix, cmap=cmap)
            axes[i].set_title(model.complete_name)
            axes[i].set_yticks(np.arange(3), np.arange(3))
            axes[i].set_xticks(np.arange(3), np.arange(3))
            axes[i].tick_params(axis='both', which='both', length=0)
            axes[i].spines['top'].set_visible(True)
            axes[i].spines['right'].set_visible(True)

            highest_value = np.max(confusion_matrix)
            for (j,k), value in np.ndenumerate(confusion_matrix):
                axes[i].text(k, j, value, ha='center', va='center', color=plot_setup.midblack if value<highest_value/2 else "white", size=20)

        elif metric_name in [metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:
            if groundtruth_hypergraph is None:
                boxplot_stats.append( get_boxplot_stats(metric_statistics[0], label=model.complete_name) )

            else:
                for hyperedge_statistics, boxplot in zip(metric_statistics, boxplot_stats):
                    boxplot.append( get_boxplot_stats(hyperedge_statistics, label=model.complete_name.replace(" ", "\n") ) )


        elif metric_name == metrics.PairwiseObservationsAverage.name:

            mean, std = metric_statistics["mean"], metric_statistics["std"]
            xvalues = np.arange(len(mean))

            ordered_observations_pairs = pygrit.get_decreasing_ordered_pairs(observations)
            ordered_observations = np.array(ordered_observations_pairs)[:, 0]

            if groundtruth_hypergraph is None:
                ax = axes[0]

                if i==0: # display observations only once
                    xmax = np.sum(ordered_observations>0)*1.2
                    ax.plot(xvalues, ordered_observations, label="Observations", color=plot_setup.midblack, zorder=0)

                    ax.set_xlabel("Ordered pairwise observations $x_{ij}$")
                    ax.set_xlim(0, xmax)

                ax.fill_between(xvalues, mean-std, mean+std, alpha=.2, color=color)
                ax.fill_between(xvalues, mean-2*std, mean+2*std, alpha=.1, color=color)
                ax.scatter(xvalues, mean, label=model.complete_name, color=color, s=5)

            else:

                real_edgetypes = np.array([groundtruth_hypergraph.get_highest_order_hyperedge_with(i, j)
                                    for _, i, j in ordered_observations_pairs])

                for ax, edgetype in zip(axes, [2, 1, 0]):
                    edgetype_idx = np.argwhere(real_edgetypes==edgetype).flatten()

                    xvalues = np.arange(len(edgetype_idx))
                    mean_of_type = mean[edgetype_idx]
                    std_of_type = std[edgetype_idx]

                    if i==0: # display observations only once
                        ax.set_xlabel("$\\ell_{ij}="+str(edgetype)+"$")
                        ax.plot(xvalues, ordered_observations[edgetype_idx],
                                color=plot_setup.midblack, zorder=0, label="Observations")

                    ax.fill_between(xvalues, mean_of_type-std_of_type, mean_of_type+std_of_type, alpha=.2, color=color)
                    ax.fill_between(xvalues, mean_of_type-2*std_of_type, mean_of_type+2*std_of_type, alpha=.1, color=color)
                    ax.scatter(xvalues, mean_of_type, label=model.complete_name, color=color, s=5)


    if metric_name in [metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:
        _colors = [plot_setup.model_colors[name]+"aa" for name in args.models]
        bxp_kwargs = dict(showfliers=False, patch_artist=True, medianprops={"color":plot_setup.midblack})
        ax_to_color = []

        if groundtruth_hypergraph is None:
            ax_to_color = [axes[0].bxp(boxplot_stats, **bxp_kwargs)]

        else:
            for ax, boxplot_stat in zip(axes, boxplot_stats):
                ax_to_color.append( ax.bxp(boxplot_stat, **bxp_kwargs) )

        for ax in ax_to_color:
            for patch, color in zip(ax["boxes"], _colors):
                patch.set_facecolor(color)
                patch.set_edgecolor("none")

    elif metric_name == metrics.PairwiseObservationsAverage.name:
        axes[0].legend()
        for edgetype, ax in enumerate(axes):
            ax.set_xticks([])

    if fig is not None:
        pyplot.tight_layout()
        if metric_name == metrics.PairwiseObservationsAverage.name:
            pyplot.subplots_adjust(wspace=.155)

        pyplot.savefig(figures_directory+f"/{dataset_name}_{metric_name}.pdf", bbox_inches='tight')
        pyplot.show()
