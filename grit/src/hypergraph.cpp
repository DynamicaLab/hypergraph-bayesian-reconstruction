#include <math.h>
#include <fstream>
#include <map>
#include <iostream>

#include <boost/filesystem.hpp>

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"


namespace GRIT {
using namespace std;


static bool stringHasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

Hypergraph::Hypergraph(size_t size) : edgeNumber(0), TriangleList(size){
    adjacencyLists.resize(size);
}

void Hypergraph::resize(size_t new_size) {
    if (new_size < size)
        throw logic_error("Hyergraph cannot be reduced in size.");

    adjacencyLists.resize(new_size);
    TriangleList::resize(new_size);  // updates "size" member
}


bool Hypergraph::addEdge(c_Index& vertex1, c_Index& vertex2) {
    return addMultiedge(vertex1, vertex2, 1);
}

bool Hypergraph::addMultiedge(c_Index& vertex1, c_Index& vertex2, size_t n) {
    if (n == 0) return false;
    if (vertex1 >= size || vertex2 >= size) throw logic_error("Adding edge to hypergraph: vertex out of range");


    Index smallestAdjacencyVertex(vertex1), otherVertex(vertex2);
    if (adjacencyLists[vertex1].size() > adjacencyLists[vertex2].size()) {
        smallestAdjacencyVertex = vertex2;
        otherVertex = vertex1;
    }

    bool _isEdge = false;

    for (auto& neighbour_multiplicity_pair: adjacencyLists[smallestAdjacencyVertex]) {
        if (neighbour_multiplicity_pair.first == otherVertex) {
            neighbour_multiplicity_pair.second+=n;
            _isEdge = true;
            break;
        }
    }
    if (!_isEdge) {
        adjacencyLists[vertex1].push_back({vertex2, n});
        adjacencyLists[vertex2].push_back({vertex1, n});
        edgeNumber++;
    }
    else {
        for (auto& neighbour_multiplicity_pair: adjacencyLists[otherVertex]) {
            if (neighbour_multiplicity_pair.first == smallestAdjacencyVertex) {
                neighbour_multiplicity_pair.second+=n;
                break;
            }
        }
    }
    return true;
}

bool Hypergraph::removeEdge(c_Index& vertex1, c_Index& vertex2) {
    if (vertex1 >= size || vertex2 >= size) throw logic_error("Removing edge from hypergraph: vertex out of range");
    bool _isEdge = false;
    auto& vertex1Neighbours = adjacencyLists[vertex1];
    auto& vertex2Neighbours = adjacencyLists[vertex2];

    for (auto it=vertex1Neighbours.begin(); it!=vertex1Neighbours.end(); it++) {
        if (it->first == vertex2) {
            _isEdge = true;

            if (it->second == 1) {
                vertex1Neighbours.erase(it--);
                edgeNumber--;
            }
            else
                it->second--;
            break;
        }
    }
    if (!_isEdge)
        return false;
    else
        for (auto it=vertex2Neighbours.begin(); it!=vertex2Neighbours.end(); it++) {
            if (it->first == vertex1) {
                if (it->second == 1)
                    vertex2Neighbours.erase(it--);
                else
                    it->second--;
                break;
            }
        }
    return true;
}

size_t Hypergraph::getHighestOrderHyperedgeWith(c_Index& vertex1, c_Index& vertex2) const {
    if (vertex1 >= size || vertex2 >= size) throw logic_error("Getting highest order hyperedge: vertex out of range");

    bool hasTriangle = isPairCovered(vertex1, vertex2);


    size_t highestOrder = 0;
    if (hasTriangle)
        highestOrder = 2;
    else if (isEdge(vertex1, vertex2))
        highestOrder = 1;

    return highestOrder;
}

size_t Hypergraph::getHighestOrderHyperedgeExcluding(c_Index& vertex1, c_Index& vertex2, const Triplet& excludedTriplet) const {
    if (vertex1 >= size || vertex2 >= size) throw logic_error("Getting highest order hyperedge: vertex out of range");

    bool hasTriangle = isPairCoveredExluding(vertex1, vertex2, excludedTriplet);


    size_t highestOrder = 0;
    if (hasTriangle)
        highestOrder = 2;
    else if (isEdge(vertex1, vertex2))
        highestOrder = 1;

    return highestOrder;
}

size_t Hypergraph::getHighestOrderHyperedgeExcluding(c_Index& vertex1, c_Index& vertex2, const Edge& excludedEdge) const {
    if (vertex1 >= size || vertex2 >= size) throw logic_error("Getting highest order hyperedge: vertex out of range");

    bool hasTriangle = isPairCovered(vertex1, vertex2);


    size_t highestOrder = 0;
    if (hasTriangle)
        highestOrder = 2;
    else if (Edge {vertex1, vertex2} != excludedEdge && isEdge(vertex1, vertex2))
        highestOrder = 1;

    return highestOrder;
}

size_t Hypergraph::getEdgeMultiplicity(c_Index& vertex1, c_Index& vertex2) const {
    if (vertex1 >= size || vertex2 >= size) throw logic_error("Getting edge multiplicity: vertex out of range");

    if (adjacencyLists[vertex1].size() <= adjacencyLists[vertex2].size()) {
        for (auto& neighbour_multiplicity_pair: adjacencyLists[vertex1])
            if (neighbour_multiplicity_pair.first == vertex2)
                return neighbour_multiplicity_pair.second;
    }
    else {
        for (auto& neighbour_multiplicity_pair: adjacencyLists[vertex2])
            if (neighbour_multiplicity_pair.first == vertex1)
                return neighbour_multiplicity_pair.second;
    }
    return 0;
}

void Hypergraph::writeToBinary(const std::string &fileName) const {
    writeTrianglesToBinary(fileName + "_triangles");
    writeEdgesToBinary(fileName + "_edges");
}

void Hypergraph::writeEdgesToBinary(const std::string &fileName) const {
    ofstream fileStream(fileName, ios::out|ios::binary);
    if (!fileStream.is_open()) throw runtime_error("The file \""+fileName+"\" could not be open to save the edge list.");


    fileStream.write((char*) &size, sizeof(size_t));
    for (Index i=0; i<size; i++) {
        for (auto& neighbour_multiplicity_pair: adjacencyLists[i]) {
            if (i < neighbour_multiplicity_pair.first) {
                fileStream.write((char*) &i, sizeof(Index));
                fileStream.write((char*) &(neighbour_multiplicity_pair.first), sizeof(Index));
                fileStream.write((char*) &(neighbour_multiplicity_pair.second), sizeof(Index));
            }
        }
    }

    fileStream.close();
}

Hypergraph Hypergraph::loadFromBinary(const std::string& filePrefix) {
    ifstream edgeFileStream(filePrefix+"_edges", ios::out|ios::binary);
    ifstream triangleFileStream(filePrefix+"_triangles", ios::out|ios::binary);

    bool hypergraphHasEdges = edgeFileStream.is_open();
    bool hypergraphHasTriangles = triangleFileStream.is_open();

    if (!hypergraphHasTriangles && !hypergraphHasEdges)
        throw logic_error("Hypergraph: no data file found using " + filePrefix);

    TriangleList __triangleList(3);
    if (hypergraphHasTriangles)
        __triangleList = TriangleList::loadFromBinary(filePrefix+"_triangles");

    size_t hypergraphSize=3;
    if (hypergraphHasTriangles)
        hypergraphSize = __triangleList.getSize();

    Hypergraph returnedGraph(hypergraphSize);

    if (hypergraphHasTriangles) {
        returnedGraph.triangles = __triangleList.getTriangles();
        returnedGraph.triangleNumber = __triangleList.getTriangleNumber();
    }

    if (hypergraphHasEdges) {
        edgeFileStream.read((char*) &hypergraphSize, sizeof(size_t));

        if (hypergraphHasTriangles)  {
            if (hypergraphSize != __triangleList.getSize())
                throw logic_error("Number of vertices in the edge and triangle files are incoherent");
        }
        else  // returnedGraph is empty and isn't of the correct size
            returnedGraph = Hypergraph(hypergraphSize);


        size_t values[3];

        while (edgeFileStream.read((char*) &values, 3*sizeof(size_t))) {
            returnedGraph.addMultiedge(values[0], values[1], values[2]);
        }
    }
    return returnedGraph;
}

const list<Triplet> Hypergraph::getFullTriangleList() const{
    list<Triplet> fullTriangleList;

    for (size_t i=0; i<size-2; i++)
        for (auto& triangleNeighbours: getTrianglesFrom(i)) {
            auto& j = triangleNeighbours.first;

            if (i < j)
                for (auto& k: triangleNeighbours.second)
                    fullTriangleList.push_back({i, j, k});
        }

    return fullTriangleList;
}

} //namespace GRIT
