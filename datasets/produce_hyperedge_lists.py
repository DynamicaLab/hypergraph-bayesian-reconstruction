import os


def find_hyperedges_of_bipartite_dataset(filename, sep=",", hyperedges_in=0, vertices_in=None):
    if vertices_in is None:
        vertices_in = not hyperedges_in

    with open(filename, 'r') as file_stream:
        hyperedges = {}
        for line in file_stream.readlines():
            if line[0].startswith('#'):
                continue

            line_values = line.split(sep)
            line_values = list(map(lambda x: x.strip(), line_values))
            hyperedge_index = line_values[hyperedges_in]
            vertex = int(line_values[vertices_in])

            if hyperedge_index not in hyperedges.keys():
                hyperedges[hyperedge_index] = set()

            hyperedges[hyperedge_index].add(vertex)
    return hyperedges


def remove_loops_and_large_hyperedges(hyperedges, k):
    return dict(filter(lambda key_val: 1<len(key_val[1])<k, hyperedges.items()))


def keep_n_vertices(hyperedges, n):
    min_index = None
    for hyperedge in hyperedges.values():
        if min_index is None:
            min_index = min(hyperedge)
        else:
            min_index = min((min_index, min(hyperedge)))

    return dict(filter(lambda x: (max(x[1])-min_index)<n, hyperedges.items()))


def write_hyperedges(filename, hyperedges):
    with open(filename, 'w') as file_stream:
        for hyperedge in hyperedges.values():
            line = ", ".join(map(str, hyperedge))
            file_stream.write(line+'\n')


dataset_files = ["plantpol.csv", "crimes.csv", "prostitution.csv", "languages.csv"]

bipartite_dir = "bipartite_graphs"
hypergraph_dir = "hypergraphs"
if not os.path.isdir(hypergraph_dir):
    os.mkdir(hypergraph_dir)

for dataset_file in dataset_files:
    complete_path = os.path.join(bipartite_dir, dataset_file)
    if not os.path.isfile(complete_path):
        raise ValueError(f"Dataset {dataset_file} not found.")

    hyperedges = find_hyperedges_of_bipartite_dataset(complete_path, sep=",")
    hyperedges = remove_loops_and_large_hyperedges(hyperedges, k=5)
    if dataset_file == "prostitution.csv":
        hyperedges = keep_n_vertices(hyperedges, 1500)
    elif dataset_file == "crimes.csv":
        hyperedges = keep_n_vertices(hyperedges, 300)

    write_hyperedges(os.path.join(hypergraph_dir, dataset_file), hyperedges)
