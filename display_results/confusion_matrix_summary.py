import os, sys
import numpy as np
from numpy.lib.function_base import percentile

sys.path.append("../")
from modeling.config import ConfigurationParserWithModels, get_json, get_dataset_name, get_config
from modeling.output import get_output_directory_for, metrics_filename
from generation.hypergraph_generation import load_binary_hypergraph

os.chdir("../")


args = ConfigurationParserWithModels().parser.parse_args()
config = get_config(args)

dataset_name = get_dataset_name(args)
output_directory = get_output_directory_for("data", dataset_name)
results = get_json( os.path.join(output_directory, metrics_filename) )

for approach_name, confusion_matrices in results.items():
    summaries = []
    for matrix in confusion_matrices:
        m = np.array(matrix).reshape(3, 3)
        number_of_interactions = np.sum(m[1:])
        summaries.append( (number_of_interactions-m[1,1]-m[2,2])/number_of_interactions )

    percentiles = [25, 50, 75]
    print(approach_name, "quartiles", [f"{x:.2f}" for x in np.percentile(summaries, percentiles)])
