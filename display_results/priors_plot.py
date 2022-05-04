import os, sys
import numpy as np
from matplotlib import pyplot
from scipy import stats

import plot_setup
sys.path.append("../")
from modeling.models import models
from utils.config import ConfigurationParserWithModels, get_config, get_dataset_name

os.chdir("../")


class BetaPrior:
    def __init__(self, parameters):
        self.distribution = lambda x: stats.beta.pdf(x, *parameters)
        self.support = [-.01, 1.01]


class GammaPrior:
    def __init__(self, parameters):
        alpha, beta = parameters
        self.distribution = lambda x: stats.gamma.pdf(x, alpha, scale=1/beta)
        self.support = [0, stats.gamma.ppf(.99, alpha, scale=1/beta)]


args = ConfigurationParserWithModels().parser.parse_args()

config = get_config(args)
dataset_name = get_dataset_name(args)
inference_models = [models[model_name](config) for model_name in args.models]


parameter_number = 5  # 5 parameters for each model
cols = 3
rows = (parameter_number // cols)+1
for model in inference_models:
    fig, axes = pyplot.subplots(rows, cols, figsize=(4*cols, 3*rows))
    axes = axes.flatten()

    hyperparameters = config["models", model.name, "hyperparameters"]
    for i, ax in enumerate(axes.flatten()):
        distribution_parameters = hyperparameters[2*i:2*(i+1)]
        if len(distribution_parameters) == 0:
            fig.delaxes(ax)
            continue

        # Distributions are always Beta, Beta, Gamma, Gamma, Gamma
        if i < 2:
            prior = BetaPrior(distribution_parameters)
        else:
            prior = GammaPrior(distribution_parameters)

        xvalues = np.linspace(*prior.support, 1000)
        ax.plot(xvalues, prior.distribution(xvalues), color=plot_setup.model_colors[model.name], lw=2)

        parameter_name = model.get_parameter_names()[i]
        ax.set_ylabel(f"$P({parameter_name})$")
        ax.set_xlabel(f"${parameter_name}$")
        ax.set_xlim(prior.support)
    pyplot.suptitle(f"Priors of model {model.name}", size=20)
    pyplot.tight_layout()
    pyplot.show()
