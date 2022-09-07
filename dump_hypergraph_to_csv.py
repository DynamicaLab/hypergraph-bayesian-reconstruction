import os

from generation.hypergraph_generation import load_binary_hypergraph
from modeling.config import GenericConfigurationParser, get_dataset_name
from modeling.output import get_output_directory_for


args = GenericConfigurationParser().parse_args()

if args.o:
    print("No ground truth hypergraph to dump for this dataset.")
    exit()

dataset_name = get_dataset_name(args)

hypergraph = load_binary_hypergraph(dataset_name)
if hypergraph is None and not args.o:
    print("No ground truth binary hypergraph file found.")
    exit()

hypergraph.write_to_csv(
        os.path.join(get_output_directory_for("data", dataset_name), "hypergraph.csv")
    )
