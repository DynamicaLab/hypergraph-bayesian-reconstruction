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
    error_proportion, f1_scores = [], []

    for matrix in confusion_matrices:
        m = np.array(matrix).reshape(3, 3)
        number_of_interactions = np.sum(m[1:])
        error_proportion.append( (number_of_interactions-m[1,1]-m[2,2])/number_of_interactions )

        true_positives = m[1,1]+m[2,2]
        false_neg_pos = number_of_interactions-m[1,1]-m[2,2]
        f1_scores.append( 2*true_positives/(2*true_positives+false_neg_pos) )

    percentiles = [25, 50, 75]
    print(approach_name,
            "\n\tError prop.\t", [f"{x:.2f}" for x in np.percentile(error_proportion, percentiles)],
            "\n\tF1 scores\t", [f"{x:.2f}" for x in np.percentile(f1_scores, percentiles)]
    )
