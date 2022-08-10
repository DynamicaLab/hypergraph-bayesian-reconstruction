import os, sys
import numpy as np
from matplotlib import pyplot
from numpy.lib.function_base import percentile
from scipy.special import gammainc, gammaincc
import warnings

import plot_setup

sys.path.append("../")
import modeling.metrics as metrics
from modeling.models import PES, PHG
from modeling.config import ConfigurationParserWithModels, get_dataset_name, get_json, get_config


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
    corrected_name = model_name
    if model_name == PES.name:
        corrected_name = "Categorical"
    elif model_name == PHG.name:
        corrected_name = "Hypergraph"

    return {
        "label": corrected_name,
        "color": plot_setup.model_colors[model_name],
        "marker": plot_setup.model_markers[model_name],
        "markersize": 4
    }


def plot_tendency(ax, xvalues, stats, **kwargs):
    ax.plot(parameter_values, stats(mid_centile), **kwargs)
    ax.fill_between(parameter_values, stats(lowest_centile), stats(highest_centile),
            alpha=.2, color=color, edgecolor="none")
    ax.fill_between(parameter_values, stats(low_centile), stats(high_centile),
            alpha=.5, color=color, edgecolor="none")


def get_article_ylabel(metric_name):
    if metric_name == metrics.SumResiduals.name:
        return "$R_k$"
    elif metric_name == "confusion summary":
        return "$\\epsilon$"
    elif metric_name == "class entropy":
        return "$S$"
    return metric_name


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

metrics_data = tendency_data["metrics"]
metric_names = set(sum([list(metrics_data[model].keys()) for model in metrics_data], []))


for metric_name in metric_names:
    if metric_name == metrics.PairwiseObservationsAverage.name:
        continue

    varied_parameter = "$\\mu_1$" if args.mu1 else "$\\mu_2$"

    fig_size = (plot_setup.fig_width, plot_setup.fig_height)
    width, height = fig_size

    if metric_name == metrics.ConfusionMatrix.name:
        fig, axes = pyplot.subplots(3, 3, figsize=(width, height*2), sharex=True)
        for i in range(3):
            axes[i, 0].set_ylabel(r"$\ell_{ij}="+str(i)+"$")
            axes[2, i].set_xlabel(varied_parameter)
            axes[0, i].set_title(r"$F(\hat \ell_{ij}="+str(i)+")$")

    elif metric_name in [metrics.SumAbsoluteResiduals.name, metrics.SumResiduals.name]:

        fig, axes = pyplot.subplots(1, 3, figsize=fig_size)
        for edgetype, ax in enumerate(axes):
            ax.set_xlabel(varied_parameter)
            ax.set_title(f"Type-{edgetype}")
            ax.tick_params(axis="both", direction='in')
        axes[0].set_ylabel(get_article_ylabel(metric_name))

    else:
        fig, axes = pyplot.subplots(1, figsize=(width/2, height))
        axes.set_xlabel(varied_parameter)
        axes.set_ylabel(get_article_ylabel(metric_name))
        axes.tick_params(axis="both", direction='in')


    for model_name in args.models:
        metric_statistics = metrics_data[model_name].get(metric_name)
        if metric_statistics is None:
            print(f"Warning: No statistics found for metric {metric_name} and model {model_name}.")
            continue

        color = plot_setup.model_colors[model_name]

        if metric_name == metrics.ConfusionMatrix.name:
            for i, ax in enumerate(axes.ravel()):
                row = i % 3
                stat = lambda stat: np.array(metric_statistics[stat])[:, i]/confusion_matrix_normalization[row][row]
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

    if metric_name == "confusion summary":
        pyplot.legend()
    fig.tight_layout()
    fig.subplots_adjust(hspace=.185, wspace=.285)
    fig.savefig(figures_directory+f"/{metric_name}_{'mu1' if args.mu1 else 'mu2'}.pdf", bbox_inches='tight')
    pyplot.close()
