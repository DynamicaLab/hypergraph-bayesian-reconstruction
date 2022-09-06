#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <vector>
#include <list>
#include <unordered_set>
#include <map>

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"


namespace py = pybind11;


double logSumExp(const std::array<double, 3>& y) {
    double ymax = y[0]>y[1] ? y[0]: y[1];
    if (y[2]>ymax)
        ymax = y[2];

    double total = 0;
    for (auto& val: y)
        total += exp(val-ymax);
    return ymax+log(total);
}

double getNegMixtureLikelihood(const py::array_t<double>& parameters,
            const py::array_t<double>& occurences,
            const py::array_t<double>& values
        ) {
    py::buffer_info parametersBuffer = parameters.request();
    double* parametersPtr = (double*) parametersBuffer.ptr;

    py::buffer_info occurencesBuffer = occurences.request();
    double* occurencesPtr = (double*) occurencesBuffer.ptr;
    double n = occurencesBuffer.shape[0];

    py::buffer_info valuesBuffer = values.request();
    double* valuesPtr = (double*) valuesBuffer.ptr;


    double loglikelihood=0;
    std::array<double, 3> elementsToSum;

    for (size_t i=0; i<n; i++) {
        for (size_t j=0; j<3; j++)
            elementsToSum[j] = log(parametersPtr[j]) + valuesPtr[i]*log(parametersPtr[3+j]) - parametersPtr[3+j] - lgamma(valuesPtr[i]+1);

        loglikelihood += occurencesPtr[i]*logSumExp(elementsToSum);
    }
    return -loglikelihood;
}


std::list<std::tuple<size_t, size_t, size_t>> getDecreasingOrderedPairs(const py::array_t<size_t>& observations) {
    py::buffer_info observationsBuffer = observations.request();
    size_t* observationsPtr = (size_t*) observationsBuffer.ptr;
    size_t n = observationsBuffer.shape[0];

    std::list<std::tuple<size_t, size_t, size_t>> orderedPairs;

    for (size_t i=0; i<n; i++)
        for (size_t j=i+1; j<n; j++)
            orderedPairs.push_back({observationsPtr[i*n+j], i, j});
    orderedPairs.sort( [](const std::tuple<size_t, size_t, size_t>& a, const std::tuple<size_t, size_t, size_t>& b) {
                return std::get<0>(a) > std::get<0>(b);
            });

    return orderedPairs;
}


GRIT::Hypergraph removeDisconnectedVertices(const GRIT::Hypergraph& hypergraph) {
    size_t previousNonEmptyVertex=0;
    std::map<size_t, size_t> newIndices;

    for (size_t i=0; i<hypergraph.getSize(); i++)
        if (hypergraph.getEdgesFrom(i).size()>0 or hypergraph.getTrianglesFrom(i).size()>0)
            newIndices[i] = previousNonEmptyVertex++;

    GRIT::Hypergraph filteredHypergraph(previousNonEmptyVertex);
    for (size_t i=0; i<hypergraph.getSize(); i++) {
        for (auto& _2neighbour: hypergraph.getEdgesFrom(i))
            if (i<_2neighbour.first)
                filteredHypergraph.addMultiedge(newIndices[i], newIndices[_2neighbour.first], _2neighbour.second);

        for (auto& first_3neighbour: hypergraph.getTrianglesFrom(i))
            if (i < first_3neighbour.first)
            for (auto& second_3neighbour: first_3neighbour.second)
                if (first_3neighbour.first < second_3neighbour)
                    filteredHypergraph.addTriangle(
                        {newIndices[i], newIndices[first_3neighbour.first], newIndices[second_3neighbour]});
    }
    return filteredHypergraph;
}


GRIT::Hypergraph projectHypergraphOnGraph(const GRIT::Hypergraph& hypergraph) {
    GRIT::Hypergraph graph(hypergraph.getSize());

    for (size_t i=0; i<hypergraph.getSize(); i++)
        for (size_t j=i+1; j<hypergraph.getSize(); j++)
            if (hypergraph.getHighestOrderHyperedgeWith(i, j) > 0)
                graph.addEdge(i, j);
    return graph;
}

GRIT::Hypergraph generateHypergraphFromAdjacencyMatrix(const py::array_t<size_t>& adjacencyMatrix) {
    py::buffer_info adjacencyBuffer = adjacencyMatrix.request();
    size_t* adjacencyPtr = (size_t*) adjacencyBuffer.ptr;
    size_t n = adjacencyBuffer.shape[0];

    GRIT::Hypergraph graph(n);

    for (size_t i=0; i<n; i++)
        for (size_t j=i+1; j<n; j++)
            if (adjacencyPtr[i*n+j])
                graph.addEdge(i, j);

    return graph;
}

GRIT::Hypergraph projectHypergraphOnMultigraph(const GRIT::Hypergraph& hypergraph) {
    GRIT::Hypergraph graph(hypergraph.getSize());

    size_t edgeMultiplicity;
    for (size_t i=0; i<hypergraph.getSize(); i++)
        for (size_t j=i+1; j<hypergraph.getSize(); j++) {
            edgeMultiplicity = hypergraph.getHighestOrderHyperedgeWith(i, j);
            if (edgeMultiplicity > 0)
                graph.addMultiedge(i, j, edgeMultiplicity);
        }
    return graph;
}

GRIT::Hypergraph getAverageHypergraph(std::list<std::string>& fileNames) {
    std::vector<std::map<std::pair<size_t, size_t>, size_t>> triangleOccurences;
    std::map<std::pair<size_t, size_t>, size_t> edgeOccurences;

    size_t sampleSize = fileNames.size();
    size_t graphSize = 0;
    for (auto& fileName: fileNames) {
        GRIT::Hypergraph hypergraph = GRIT::Hypergraph::loadFromBinary(fileName);
        if (graphSize == 0) {
            graphSize = hypergraph.getSize();
            triangleOccurences.resize(graphSize);
        }
        else
            if (graphSize != hypergraph.getSize())
                throw std::logic_error("Hypergraph average: a hypergraph has size "+std::to_string(graphSize)+
                        " while another has size "+std::to_string(hypergraph.getSize()));

        for (size_t i=0; i<hypergraph.getSize(); i++) {

            for (auto& j: hypergraph.getEdgesFrom(i)) {
                if (i >= j.first)
                    continue;

                if (edgeOccurences.find({i, j.first}) == edgeOccurences.end())
                    edgeOccurences[{i, j.first}] = 1;
                else
                    edgeOccurences[{i, j.first}]++;

            }

            for (auto& triangleNeighbour: hypergraph.getTrianglesFrom(i)) {
                auto& j = triangleNeighbour.first;
                if (i >= j)
                    continue;

                for (auto& k: triangleNeighbour.second) {
                    std::pair<size_t, size_t> edge = {j, k};
                    if (triangleOccurences[i].find(edge) == triangleOccurences[i].end())
                        triangleOccurences[i][edge] = 1;
                    else
                        triangleOccurences[i][edge] ++;
                }
            }
        }
    }

    GRIT::Hypergraph averageHypergraph(graphSize);
    for (auto& edge: edgeOccurences)
        if ((double) edge.second/sampleSize >= 0.5)
            averageHypergraph.addEdge(edge.first.first, edge.first.second);

    for (size_t i=0; i<triangleOccurences.size(); i++)
        for (auto& neighbours: triangleOccurences[i])
            if ((double) neighbours.second/sampleSize >= 0.5)
                averageHypergraph.addTriangle({i, neighbours.first.first, neighbours.first.second});
    return averageHypergraph;
}


size_t getEdgeHammingDistance(const GRIT::Hypergraph& hypergraph1, const GRIT::Hypergraph& hypergraph2){
    size_t hammingDistance = 0;
    if (hypergraph1.getSize() != hypergraph2.getSize()) throw std::logic_error("Edge hamming distance: hypergraphs have different sizes");

    size_t hyperedgeType1, hyperedgeType2;
    for (size_t i=0; i<hypergraph1.getSize(); i++) {
        for (size_t j=i+1; j<hypergraph1.getSize(); j++) {
            hyperedgeType1 = hypergraph1.getEdgeMultiplicity(i, j);
            hyperedgeType2 = hypergraph2.getEdgeMultiplicity(i, j);
            if ( hyperedgeType1 != hyperedgeType2 )
                hammingDistance++;
        }
    }
    return hammingDistance;
}

size_t getGlobalHammingDistance(const GRIT::Hypergraph& hypergraph1, const GRIT::Hypergraph& hypergraph2){
    size_t hammingDistance = 0;
    if (hypergraph1.getSize() != hypergraph2.getSize()) throw std::logic_error("Edge hamming distance: hypergraphs have different sizes");

    size_t hyperedgeType1, hyperedgeType2;
    for (size_t i=0; i<hypergraph1.getSize(); i++) {
        for (size_t j=i+1; j<hypergraph1.getSize(); j++) {
            hyperedgeType1 = hypergraph1.getHighestOrderHyperedgeWith(i, j);
            hyperedgeType2 = hypergraph2.getHighestOrderHyperedgeWith(i, j);
            if ( hyperedgeType1 != hyperedgeType2 )
                hammingDistance++;
        }
    }
    return hammingDistance;
}


void defineAdditionalUtils(py::module &m) {
    m.def("get_neg_mixture_likelihood", &getNegMixtureLikelihood);
    m.def("get_decreasing_ordered_pairs", &getDecreasingOrderedPairs);

    m.def("get_edge_hamming_distance", &getEdgeHammingDistance);
    m.def("get_global_hamming_distance", &getGlobalHammingDistance);

    m.def("remove_disconnected_vertices", &removeDisconnectedVertices);
    m.def("project_hypergraph_on_multigraph", &projectHypergraphOnMultigraph);
    m.def("project_hypergraph_on_graph", &projectHypergraphOnGraph);
    m.def("generate_hypergraph_from_adjacency", &generateHypergraphFromAdjacencyMatrix);
    m.def("get_average_hypergraph", &getAverageHypergraph);
}
