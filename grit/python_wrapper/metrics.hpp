#include <vector>
#include <pybind11/numpy.h>

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"


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
