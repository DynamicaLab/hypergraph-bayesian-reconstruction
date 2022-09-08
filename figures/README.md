## Steps to reproduce the results

After installing the `pygrit` module, the following commands were used to generate the figures shown in the [article](https://arxiv.org/abs/2208.06503). The sequence of commands are ran from the root of this project.

Because some scripts produce a large number of files and super computing facilities impose a maximum number of files, the output of the algorithms can be moved to another location. This is set in the variable `main_output_directory` in ``modeling/output.py)``.

The synthetic hypergraphs (e.g. SBM, CM, $\beta$-model) used here are fixed in order to reproduce the results. However, one can generate new hypergraphs with these models. For example, for the hypergraph CM model, use
```
python generate_data.py -s miller-cm.json
```
and use the same tools to produce and analyze.


### Fig. 3

```
python generate_data.py -g karate.json
python sample.py pes phg -g karate.json
cd display_results/hypergraph_figures
python average_structure.py pes phg -g karate.json
```

### Table I

For each dataset (`karate`, `crimes`, `prostitution`, `plantpol`, `languages`, `fixed_sbm`, `fixed_cm`, `fixed_beta`, `fixed_best`, `fixed_worst`), run
```
python generate_data.py -g <dataset>.json
python confusion_matrix_dataset.py pes phg -g <dataset>.json
cd display_results
python hypergraph_info.py -g <dataset>.json
python confusion_matrix_summary.py pes phg -g <dataset>.json
```

### Fig. 5 and Fig. 9

```
python generate_data.py -g fixed_best.json
mpiexec -np 100 python tendency_fixed_hypergraph.py pes phg -g fixed_best.json --mu1
cd display_results
python gen_tendency_data.py pes phg -g fixed_best.json --mu1
python plot_tendency_data.py pes phg -g fixed_best.json --mu1
```

### Fig. 6 and Fig. 10

```
python generate_data.py -g fixed_worst.json
mpiexec -np 100 python tendency_fixed_hypergraph.py pes phg -g fixed_worst.json --mu1
cd display_results
python gen_tendency_data.py pes phg -g fixed_worst.json --mu1
python plot_tendency_data.py pes phg -g fixed_worst.json --mu1
```

### Fig. 7

```
python generate_data.py -g fixed_best.json
mpiexec -np 100 python tendency_fixed_hypergraph.py pes phg -g fixed_best.json --mu2
cd display_results
python gen_tendency_data.py pes phg -g fixed_best.json --mu2
python plot_tendency_data.py pes phg -g fixed_best.json --mu2
```

### Fig. 8

```
python generate_data.py -g fixed_worst.json
mpiexec -np 100 python tendency_fixed_hypergraph.py pes phg -g fixed_worst.json --mu2
cd display_results
python gen_tendency_data.py pes phg -g fixed_worst.json --mu2
python plot_tendency_data.py pes phg -g fixed_worst.json --mu2
```
