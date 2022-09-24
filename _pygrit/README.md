# GrIT: Graph Inference of Triangles

CMake and Boost are required in order to compile the Python and C++ libraries.

### Install the Python wrapper module
```
pip install .
```

### Build unit tests
The unit tests require GTest.
```
    mkdir build && cd build
    cmake -DBUILD_TESTS=ON ..
    make
```
