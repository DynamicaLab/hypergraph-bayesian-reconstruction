#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <map>
#include <vector>

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"


namespace py = pybind11;


void addEdgeIfInexistent(size_t i, size_t j, std::map<std::pair<size_t, size_t>, bool>& edgeInTriangle) {
    auto edge = std::make_pair(i, j);
    if (!edgeInTriangle.count(edge))
        edgeInTriangle[edge] = 1;
}

size_t countEdgesInTriangles(const py::array_t<size_t>& edgeTypes) {
    py::buffer_info edgeTypesBuffer = edgeTypes.request();
    size_t* L = (size_t*) edgeTypesBuffer.ptr;
    size_t n = edgeTypesBuffer.shape[0];

    std::map<std::pair<size_t, size_t>, bool> edgeInTriangle;
    size_t Lij, Lik, Ljk;

    for (size_t i=0; i<n; i++) {
        for (size_t j=i+1; j<n; j++) {
            Lij = L[i*n + j];
            if (Lij == 0)
                continue;

            for (size_t k=j+1; k<n; k++) {
                Lik = L[i*n + k];
                Ljk = L[j*n + k];
                if (Lik>0 && Ljk>0) {
                    if (Lij==1)
                        addEdgeIfInexistent(i, j, edgeInTriangle);
                    if (Lik==1)
                        addEdgeIfInexistent(i, k, edgeInTriangle);
                    if (Ljk==1)
                        addEdgeIfInexistent(j, k, edgeInTriangle);
                }
            }
        }
    }
    return edgeInTriangle.size();
}


std::vector<size_t> getConfusionMatrix(const GRIT::Hypergraph& groundtruth, const py::array_t<size_t>& averageHyperedgeTypes, bool withCorrelation, bool edgetype_swapped) {
    py::buffer_info averageTypesBuffer = averageHyperedgeTypes.request();
    size_t* averageTypesPtr = (size_t*) averageTypesBuffer.ptr;
    size_t n = averageTypesBuffer.shape[0];

    std::vector<size_t> confusionMatrix(9, 0);

    size_t averageType, groundtruthType;
    for (size_t i=0; i<n; i++) {
        for (size_t j=i+1; j<n; j++) {
            averageType = averageTypesPtr[i*n+j];
            if (withCorrelation)
                groundtruthType = groundtruth.getHighestOrderHyperedgeWith(i, j);
            else
                groundtruthType = groundtruth.getEdgeMultiplicity(i, j);

            if (edgetype_swapped) {
                if (averageType == 1)
                    averageType = 2;
                else if (averageType == 2)
                    averageType = 1;
            }

            confusionMatrix[groundtruthType*3 + averageType]++;
        }
    }
    return confusionMatrix;
}

std::array<double, 3> getSumOfResidualsOfTypes(const py::list& edgetypes, const py::array_t<size_t>& X1, const py::array_t<size_t>& X2) {
    py::buffer_info X1_buffer = X1.request();
    py::buffer_info X2_buffer = X2.request();

    if (X1_buffer.size != X2_buffer.size)
        throw std::runtime_error("Sum of residuals: Input shapes must match.");

    size_t n = X1_buffer.shape[0];

    size_t *X1_ptr = (size_t *) X1_buffer.ptr,
           *X2_ptr = (size_t *) X2_buffer.ptr;

    std::array<double, 3> residuals = {0, 0, 0};
    size_t totalPosition = 0;
    for (size_t i=0; i<n; i++)
        for (size_t j=i+1; j<n; j++) {
            residuals[edgetypes[totalPosition].cast<size_t>()] +=
                (double)X1_ptr[i*n+j] - (double)X2_ptr[i*n+j];
            totalPosition++;
        }

    return residuals;
}

std::array<double, 3> getSumOfAbsoluteResidualsOfTypes(const py::list& edgetypes, const py::array_t<size_t>& X1, const py::array_t<size_t>& X2) {
    py::buffer_info X1_buffer = X1.request();
    py::buffer_info X2_buffer = X2.request();

    if (X1_buffer.size != X2_buffer.size)
        throw std::runtime_error("Sum of residuals: Input shapes must match.");

    size_t n = X1_buffer.shape[0];

    size_t *X1_ptr = (size_t *) X1_buffer.ptr,
           *X2_ptr = (size_t *) X2_buffer.ptr;

    std::array<double, 3> residuals = {0, 0, 0};
    size_t totalPosition = 0;
    for (size_t i=0; i<n; i++)
        for (size_t j=i+1; j<n; j++) {
            residuals[edgetypes[totalPosition].cast<size_t>()] +=
                std::abs((double)X1_ptr[i*n+j] - (double)X2_ptr[i*n+j]);
            totalPosition++;
        }

    return residuals;
}

void defineMetrics(py::module &m) {
    m.def("count_edges_in_triangles", &countEdgesInTriangles);

    m.def("get_confusion_matrix", &getConfusionMatrix,
            py::arg("groundtruth"), py::arg("average edge types"),
            py::arg("with_correlation")=false, py::arg("edge_types_swapped")=false);
    m.def("get_sum_residuals_of_types", &getSumOfResidualsOfTypes,
            py::arg("edge_types"), py::arg("observations1"), py::arg("observations2"));
    m.def("get_sum_absolute_residuals_of_types", &getSumOfAbsoluteResidualsOfTypes,
            py::arg("edge_types"), py::arg("observations1"), py::arg("observations2"));
}
