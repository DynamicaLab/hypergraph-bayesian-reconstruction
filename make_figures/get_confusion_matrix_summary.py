import os, sys
import numpy as np
from numpy.lib.function_base import percentile

sys.path.append("../")
from utils.config import ConfigurationParserWithModels, get_json, get_dataset_name, get_config
from utils.output import get_output_directory_for, metrics_filename
from modelling.models import models
from generation.hypergraph_generation import load_binary_hypergraph

os.chdir("../")


args = ConfigurationParserWithModels().parser.parse_args()
config = get_config(args)
inference_models = [models[model_name](config) for model_name in args.models]

dataset_name = get_dataset_name(args)
confusion_matrices = get_json( os.path.join(get_output_directory_for("data", dataset_name), metrics_filename) )

for model_name in args.models:
    print(model_name)
    summaries = []
    for matrix in confusion_matrices[model_name]:
        summaries.append(np.sum(matrix) - np.trace(np.array(matrix).reshape(3, 3)))
    print(summaries)

