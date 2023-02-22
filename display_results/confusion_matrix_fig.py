import os, sys
import numpy as np
from numpy.lib.function_base import percentile
from matplotlib import pyplot, rcParams, lines, use

sys.path.append("../")
from modeling.config import ConfigurationParserWithModels, get_json, get_config_from_str
from modeling.output import get_output_directory_for, metrics_filename
from generation.hypergraph_generation import load_binary_hypergraph
import plot_setup


os.chdir("../")

datasets = [
    "karate",
    "crimes",
    "prostitution",
    "plantpol",
    "languages",
    "fixed_sbm",
    "fixed_cm",
    "fixed_beta",
    "fixed_best",
    "fixed_worst",
]

e_delta = lambda x: f"\n($E_\\Delta={x}$)"
dataset_clean_name = {
    "karate": "Zachary's"+e_delta(0),
    "crimes": "Crimes"+e_delta(0),
    "prostitution": "Sexual contacts"+e_delta(0),
    "plantpol": "Plant-pollinator"+e_delta(0.8),
    "languages": "Languages"+e_delta(0),
    "fixed_sbm": "Hyper. SBM"+e_delta(0.2),
    "fixed_cm": "Hyper. CM"+e_delta(0.32),
    "fixed_beta": "$\\beta$-model"+e_delta(0.7),
    "fixed_best": "Best-case"+e_delta(0),
    "fixed_worst": "Worst-case"+e_delta(1)
}

approaches_clean_name = {
    "pes": "Categorical",
    "phg": "Hypergraph",
    "2step-bayesian": "Bayesian$\\times$2",
    "edge-threshold": "Threshold\n+Bayesian",
    "threshold": "Threshold$\\times$2"
}

get_line_color = lambda approach: plot_setup.model_colors.get(approach, plot_setup.lightgray)
get_marker_color = lambda approach: plot_setup.dark_model_colors.get(approach, plot_setup.midblack)

approach_markers = plot_setup.model_markers
approach_markers["2step-bayesian"] = "x"
approach_markers["edge-threshold"] = "+"
approach_markers["threshold"] = "*"

rcParams["axes.spines.left"] = False

fig, ax = pyplot.subplots(1, figsize=(5, 8))


use_f1_scores = False

y_inner_sep = .3
y_outer_sep = 1
y_pos = 0
y_tick_pos = y_pos+.5*y_inner_sep
y_tick_labels = []
y_tick_values = []


approaches = ["pes", "phg", "2step-bayesian", "edge-threshold", "threshold"]
for i, dataset_name in enumerate(datasets):
    config = get_config_from_str(dataset_name+".json", "g")

    output_directory = get_output_directory_for("data", dataset_name)
    results = get_json( os.path.join(output_directory, metrics_filename) )

    for approach_name in approaches:
        error_proportion, f1_scores = [], []

        for matrix in results[approach_name]:
            m = np.array(matrix).reshape(3, 3)
            number_of_interactions = np.sum(m[1:])
            error_proportion.append( (number_of_interactions-m[1,1]-m[2,2])/number_of_interactions )

            true_positives = m[1,1]+m[2,2]
            false_neg_pos = number_of_interactions-m[1,1]-m[2,2]
            f1_scores.append( 2*true_positives/(2*true_positives+false_neg_pos) )

        percentiles = [25, 50, 75]

        res = np.percentile(1-np.array(f1_scores), percentiles) if use_f1_scores else\
                np.percentile(error_proportion, percentiles)

        res = [r if r>1e-3 else 1e-3 for r in res]

        ax.errorbar(
                res[1], y_pos, xerr=[[res[1]-res[0]], [res[2]-res[1]]],
                ecolor=get_line_color(approach_name), color=get_marker_color(approach_name),
                marker=approach_markers[approach_name],
                markersize=5 if approach_name != "phg" else 4,
            )

        y_tick_pos -= .5*y_inner_sep
        y_pos -= y_inner_sep

    y_pos += y_inner_sep # remove effect of last inner_sep

    y_tick_labels.append(dataset_clean_name[dataset_name])
    y_tick_values.append(y_tick_pos)

    if i < len(datasets)-1:
        ax.axhline(y_pos-.5*y_outer_sep, ls="--", color="#e0e0e0")

    y_pos -= y_outer_sep
    y_tick_pos = y_pos+.5*y_inner_sep


ax.set_ylim((y_tick_pos, None))

ax.legend(
    [lines.Line2D([], [], color=plot_setup.model_colors.get(name, plot_setup.lightgray),
                  markerfacecolor=get_marker_color(name), markeredgecolor=get_marker_color(name),
                  marker=approach_markers[name])
        for name in approaches],
    [approaches_clean_name[name] for name in approaches],
    #loc=(0.5, 0.),
    bbox_to_anchor=(0.8, -.1),
    ncol=2,
    fontsize=12
)

ax.set_yticks(y_tick_values, y_tick_labels)
ax.tick_params(axis='y', which='both', length=0, pad=15)
ax.set_xlabel("$1-$F${}_1$-score" if use_f1_scores else "Error $\\epsilon$", size=14)
ax.set_xscale("log")
ax.set_xlim((None, 1.))

pyplot.tight_layout()
pyplot.savefig(os.path.join(os.getcwd(), "figures", "confusion_vis.pdf"))
pyplot.show()
