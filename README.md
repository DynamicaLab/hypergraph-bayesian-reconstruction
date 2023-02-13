# Hypergraph bayesian reconstruction

This project provides a command-line interface (CLI) to generate synthetic observations and hypergraphs, generate plausible multiplex graphs and hypergraphs for an observation matrix and compute metrics on the produced inference.

Implementation of the algorithms is done in C++ to maximize efficiency, but a Python interface is provided to facilitate its usage. The workings of the algorithms are explained in the article: https://arxiv.org/abs/2208.06503. A guide to reproduce its figures is provided [here](figures/README.md).


## Requirements

Core C++ library:
- C++17 compiler
- [CMake]
- [Boost]

Python module installation:
- [pybind11]
- [scikit-build]
- [Mpi4py]

## Installation

This project depends on [SamplableSet], so it must be installed recursively using
```
git clone --recurse-submodules https://github.com/DynamicaLab/hypergraph-bayesian-reconstruction.git
```
or
```
git clone https://github.com/DynamicaLab/hypergraph-bayesian-reconstruction.git
git submodule update --init --recursive
```
The SamplableSet library needs to be compiled:
```
cd hypergraph-bayesian-reconstruction/_pygrit/include/SamplableSet/src
mkdir build
cd build
cmake ..
make
```
This project was developped under Linux in which static libraries have the ".a" extension. Depending on the operating system, it might be necessary before proceeding to change the ".a" extension in "_pygrit/setup.py" (e.g. Windows uses ".lib"). The compiled library file name is required in the parameter `extra_objects=` of the `Extension` instantiation.

Finally, the ``_pygrit`` Python module can be installed
```
cd hypergraph-bayesian-reconstruction/_pygrit
pip install .
```

## Config files

The inference algorithms are configured in configuration files located in the `config/` directory. These files allow for simplicity, flexibility and readability by combining the configuration of the algorithms in a rather small file. In order to reduce the redundancy of config files, default values are provided in `config/default.json`. If a parameter is not specified in the config file of a dataset, the default value is used.

For each dataset analyzed, a configuration file must be created in one of three directories depending on its use case:
 - `config/synthetic/` [``-s``]: Analyze synthetic observations generated from a synthetic hypergraph
 - `config/graph-data/` [``-g``]: Analyze synthetic observations generated from a known network
 - `config/observation-data/` [``-o``]: Use an observation matrix

The flags (``-s``, ``-g``, ``-o``) identify which kind of analysis is performed in the scripts. To know what parameters can be tweaked in the configuration file, one can look at the default config file.

Note that some parameters are required. For the ``-s`` flag
 - `vertex number`: number of vertices in the network

For the ``-g`` flag
 - `dataset`: path to the dataset (absolute or relative to the root of the project)
 - `data format`: network format (only `"hyperedge list"` is currently supported)
 - `sep`: separation characters in hyperedge list

For the ``-o`` flag
 - `vertex number`: number of vertices in the network
 - `dataset`: path to the dataset (absolute or relative to the root of the project)
 - `data format`: observations format (`"csv matrix"`, `"csv edgelist"` or `"csv weighted timeseries"`)

Important note: when adding a new config file, make sure that its name is **unique across all the directories**. Otherwise, it will erase the other identical dataset.

## Models

Three inference models are available: a hypergraph model, a categorical-graph model and a standard G(n, p) model. These models are refered as "phg" (**P**oisson **H**yper**g**raph model), "pes" (**P**oisson **E**dge **S**trength model) and "per" (**P**oisson **E**rdos-**R**enyi model).


## Script execution

Most scripts are ran using
```bash
python script.py models [models...] ( -s | -g | -o ) config
```
In this command, ``models`` are the models to process using the aforementionned names (i.e. "phg", "pes", "per"), flags ``-s``, ``-g`` and ``-o`` represent the type of analysis and ``config`` is the name of the config file.

Because some scripts are independent of the model (e.g. `generate_data.py` and `display_results/hypergraph_info.py`) their signature omits the ``models`` argument
```bash
python script.py ( -s | -g | -o ) config
```
The only exception to these rules is `tendency_fixed_hypergraph.py`: the varied parameter (µ<sub>1</sub> or µ<sub>2</sub>) must be specified
```bash
python tendency_fixed_hypergraph.py models [models...] ( -s | -g | -o ) config ( --mu1 | --mu2 )
```

See next section for usage examples.

## Example: Zachary's karate club dataset

Here are the steps to produce a sample of Zachary's karate club reconstruction and to analyze it.

Because the network is known, the appropriate location to place the config file is `config/graph-data/karate.json`. The flag is then ``-g``.

Before sampling, synthetic observations must be generated. The algorithm and parameters used to do so are specified in the config files. In order to produce this dataset, run
```bash
python generate_data.py -g karate.json
```
This script serves the puropse of transforming hypergraphs and observations to a binary format, which are used by other scripts. This step is mendatory for any dataset.

The histogram of the pairwise observations can now be displayed with
```bash
cd display_results
python observations_distribution.py -g karate.json
```
and the hypergraph information with
```bash
python hypergraph_info.py -g karate.json
```
To sample the structure and parameters from this dataset for the categorical-graph model and the hypergraph model, run
```bash
cd ..
python sample.py pes phg -g karate.json
```
To display the parameters marginal distributions of the sample
```bash
cd display_results
python parameters_marginals.py pes phg -g karate.json
```
To view an animation of sample of structures generated and the structure estimators, run
```bash
cd hypergraph_figures
python sample_animation.py pes phg -g karate.json
python average_structure.py pes phg -g karate.json
```
To obtain the computed metrics on the sample, run
```bash
cd ..
python inference_metrics.py pes phg -g karate.json
```

## Inference tendency and parallel computation

The scripts `tendency_fixed_hypergraph.py` and `confusion_matrix_dataset.py` are slightly different from the others because they apply only on datasets with a known network structure (i.e. `-g` or `-s`) and they don't rely on the observations generated by `generate_data.py`.

Because they require a substantial number of simulations, MPI tools are provided to run multiple simulations in parallel. This is done using the [Mpi4py] library. To run a script in parallel, use
```bash
mpiexec -np N python script.py ...
```
where `N` is the number of parallel processes and `...` are the parameters of the script.

[Boost]: https://www.boost.org
[CMake]: https://cmake.org
[pybind11]: https://pybind11.readthedocs.io
[SamplableSet]: https://github.com/gstonge/SamplableSet
[Mpi4py]: https://mpi4py.readthedocs.io
[scikit-build]: https://scikit-build.readthedocs.io/en/latest/