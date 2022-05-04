import os, sys
import numpy as np
from matplotlib import pyplot, lines, patches, rcParams
from scipy import stats

import plot_setup
sys.path.append("../")
from modeling.models import models
from utils.config import ConfigurationParserWithModels, get_config, get_dataset_name
from utils.output import get_output_directory_for, get_sample

os.chdir("../")


args = ConfigurationParserWithModels().parser.parse_args()

dataset_name = get_dataset_name(args)
config = get_config(args)
inference_models = [models[model_name](config) for model_name in args.models]

sample_size = config["sampling", "sample size"]
parameter_number = 5  # 5 parameters for each model


rcParams['axes.spines.right'] = True
rcParams['axes.spines.top'] = True


figures_directory = plot_setup.get_figure_dir(dataset_name)
if not os.path.isdir(figures_directory):
    os.mkdir(figures_directory)


for model in inference_models:
    parameter_sample = None
    sample_location = get_output_directory_for("inference", dataset_name, model.name)

    for hypergraph, parameters in get_sample(sample_location, sample_size):
        if parameter_sample is None:
            parameter_sample = parameters
        else:
            parameter_sample = np.append(parameter_sample, parameters)


    if parameter_sample is None:
        print(f"No parameter sample found for {model.name}.")
        continue

    parameter_sample = parameter_sample.reshape(-1, parameter_number).T

    rx, ry = 0.1, 0.1
    widths = [1.]*parameter_number + [parameter_number*rx/(1-rx)]
    heights = [parameter_number*ry/(1-ry)] + [1.]*parameter_number

    fig, axes = pyplot.subplots(nrows=parameter_number, ncols=parameter_number,
                                gridspec_kw={"wspace":0.025, "hspace":0.05},
                                figsize=(10, 10))
    color = plot_setup.model_colors[model.name]
    darkcolor = plot_setup.dark_model_colors[model.name]


    for row in range(parameter_number):
        parameter_name = f"${model.get_parameter_names()[row]}$"

        axes[parameter_number-1, row].set_xlabel(parameter_name)
        axes[row, 0].set_ylabel(parameter_name)

        for col in range(parameter_number):
            if row < col:
                # pearson_correlation = stats.pearsonr(parameter_sample[col], parameter_sample[row])[0]
                # axes[row, col].text(0.5, 0.5,
                            # f"r={pearson_correlation:.2f}",
                            # va="center", ha="center")
                pyplot.delaxes(axes[row, col])
            else:
                if row == col:
                    axes[row, col].hist(parameter_sample[col], bins=20, color=color+"77", edgecolor=darkcolor)
                else:
                    axes[row, col].scatter(parameter_sample[col], parameter_sample[row],
                            s=10, alpha=0.5, color=color, edgecolor="none")

                if col > 0:
                    if row != col:
                        axes[row, col].sharey(axes[row, col-1])
                    axes[row, col].tick_params(left=False)

                if row < parameter_number-1:
                    axes[row, col].sharex(axes[row+1, col])
                    axes[row, col].tick_params(bottom=False)


    for ax in fig.get_axes():
        ax.label_outer()


    pyplot.savefig(figures_directory+f"/{model.name}_marginals_{dataset_name}.pdf", bbox_inches='tight')
    pyplot.show()
