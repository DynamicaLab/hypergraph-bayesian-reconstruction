# GrIT: Graph Inference of Triangles

CMake and Boost are required in order to compile the Python and C++ libraries.

### Install the Python wrapper module
```
    mkdir build && cd build
    cmake -DBUILD_PYLIB=ON ..
    make
    pip install python_wrapper/grit
```
The last command might not work if the Python installation is in a protected directory. If this is the case, two options are possible:
1. install the library inside a virtual environment or
2. run the command as administrator on Windows or insert ``sudo`` before the command on UNIX-like and MacOS systems.

### Uninstalling the Python module
Uninstalling the library requires a bit more work. The simplest way is to reinstall and record the installed files to remove them after:
```
    python setup.py install --record tmp_installation_files.txt
```
On UNIX-like and MacOS, use ``xargs``
```
    xargs rm -rf < tmp_installation_files.txt
```
and on Windows  use Powershell:
```
    Get-Content tmp_installation_files.txt | ForEach-Object {Remove-Item $_ -Recurse -Force}
```
The file containing the installation can be removed afterwards
```
    rm tmp_installation_files.txt
```

### Build unit tests
The unit tests require GTest.
```
    mkdir build && cd build
    cmake -DBUILD_TESTS=ON ..
    make
```

TODO:
Move the parameter sampler and the priors into the graph and data models
