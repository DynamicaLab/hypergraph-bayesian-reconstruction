import os, sys
import numpy as np
from matplotlib import pyplot
from numpy.lib.function_base import percentile
from scipy.special import gammainc, gammaincc
import warnings

import plot_setup

sys.path.append("../")
import modeling.metrics as metrics
from modeling.models import models
from modeling.config import ConfigurationParserWithModels, get_json, get_config


os.chdir("../")


lowest_centile = "centile0.025"
low_centile = "centile0.25"
high_centile = "centile0.75"
highest_centile = "centile0.975"
mid_centile = "centile0.5"


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


def get_model_kwargs(model_name):
    return {
        "label": models[model_name].name,
        "color": plot_setup.model_colors[model_name],
        "marker": plot_setup.model_markers[model_name]
    }


def plot_tendency(ax, xvalues, stats, **kwargs):
    ax.plot(parameter_values, normalized_stat(mid_centile), **kwargs)
    ax.fill_between(parameter_values, normalized_stat(lowest_centile), normalized_stat(highest_centile), alpha=.1, color=color)
    ax.fill_between(parameter_values, normalized_stat(low_centile), normalized_stat(high_centile), alpha=.2, color=color)


args = TendencyParser().parser.parse_args()
if args.o:
    raise ValueError("Observation datasets cannot be used to make this analysis.")

config = get_config(args)
dataset_name = get_dataset_name(args)
figures_directory = plot_setup.get_figure_dir(dataset_name)

tendency_data = get_json(os.path.join(
                        figures_directory,
                        f"tendency_data_mu{'1' if args.mu1 else '2'}.json"
                    ))
confusion_matrix_normalization = tendency_data["confusion normalization"]

if args.mu1:
    parameter_values = config["tendency", "varying mu1", "mu1"]
elif args.mu2:
    parameter_values = config["tendency", "varying mu2", "mu2"]


for metric_name in tendency_data["metrics"]:
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
        metric_statistics = tendency_data["metrics"][model_name].get(metric_name)
        color = plot_setup.model_colors[model_name]

        if metric_name == metrics.ConfusionMatrix.name:
            for i, ax in enumerate(axes.ravel()):
                row = i % 3
                stat = lambda stat: np.array(metric_statistics[stat])[:, i]/confusion_matrix_normalization[row, row]
                plot_tendency(ax, parameter_values, stat,
                                **get_model_kwargs(model_name), clip_on=False)
                ax.set_ylim(-0.05, 1.05)


        elif metric_name in [metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:
            for edgetype, ax in enumerate(axes):
                stat = lambda name: np.array(metric_statistics[name])[:, edgetype]
                plot_tendency(ax, parameter_values, stat, **get_model_kwargs(model_name))

        else:
            stat = lambda name: metric_statistics[name]
            plot_tendency(axes, parameter_values, stat, **get_model_kwargs(model_name))

    pyplot.legend()
    fig.tight_layout()
    fig.subplots_adjust(hspace=.185, wspace=.285)
    fig.savefig(figures_directory+f"/{metric_name}_{'mu1' if args.mu1 else 'mu2'}.pdf", bbox_inches='tight')
    pyplot.close()
