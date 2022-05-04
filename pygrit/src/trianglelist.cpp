#include <stdexcept>
#include <fstream>
#include <list>
#include <algorithm>
#include <iostream>

#include "GRIT/trianglelist.h"


namespace GRIT {
using namespace std;


TriangleList::TriangleList(size_t size): size(size), triangleNumber(0) {
    if (size < 3)
        throw logic_error("There must be at least 3 vertices in the triangle list");
    triangles.resize(size);
}

void TriangleList::resize(size_t new_size) {
    if (new_size < size)
        throw logic_error("Triangle list cannot be reduced in size.");

    size = new_size;
    triangles.resize(size);
}

static bool addTriangleNeighbour(AdjacentTriangles& triangles, const Index& j, const Index& k) {
    bool added;

    if (triangles.find(j) == triangles.end()) {
        triangles[j] = {k};
        added = true;
    }
    else
        added = triangles[j].insert(k).second;

    return added;
}


bool TriangleList::addTriangle(const Triplet& triplet){
    Triplet orderedTriplet = triplet.getOrdered();
    const size_t& i = orderedTriplet.i;
    const size_t& j = orderedTriplet.j;
    const size_t& k = orderedTriplet.k;

    if (i==j || i==k || j==k)
        return false;


    bool added = addTriangleNeighbour(triangles[i], j, k);

    if (added) {
        addTriangleNeighbour(triangles[j], i, k);
        addTriangleNeighbour(triangles[k], i, j);
        triangleNumber++;
    }
    return added;
}

static bool removeTriangleNeighbour(AdjacentTriangles& triangles, const Index& j, const Index& k) {
    bool removed;

    if (triangles.find(j) == triangles.end())
        removed = false;
    else {
        removed = triangles[j].erase(k) > 0;

        if (triangles[j].size() == 0)
            triangles.erase(j);
    }
    return removed;
}


bool TriangleList::removeTriangle(const Triplet& triplet){
    Triplet orderedTriplet = triplet.getOrdered();
    const size_t& i = orderedTriplet.i;
    const size_t& j = orderedTriplet.j;
    const size_t& k = orderedTriplet.k;

    if (i==j || i==k || j==k)
        return false;

    bool removed = removeTriangleNeighbour(triangles[i], j, k);

    if (removed) {
        removeTriangleNeighbour(triangles[j], i, k);
        removeTriangleNeighbour(triangles[k], i, j);
        triangleNumber--;
    }
    return removed;
}

bool TriangleList::isTriangle(const Triplet& triplet) const {
    Triplet orderedTriplet = triplet.getOrdered();
    auto& triangleNeighbours = getTrianglesFrom(orderedTriplet.i);

    if (triangleNeighbours.find(orderedTriplet.j) == triangleNeighbours.end())
        return false;

    auto& trianglesFromPair = triangleNeighbours.at(orderedTriplet.j);
    return trianglesFromPair.find(orderedTriplet.k) != trianglesFromPair.end();
}

bool TriangleList::isPairCovered(const Index& i, const Index& j) const {
    size_t ordered_i = i<j? i : j;
    size_t ordered_j = i<j? j : i;


    auto& triangleNeighbours_i = getTrianglesFrom(ordered_i);
    bool covered = triangleNeighbours_i.find(ordered_j) != triangleNeighbours_i.end();

    if (!covered) {
        auto& triangleNeighbours_j = getTrianglesFrom(ordered_j);
        covered = triangleNeighbours_j.find(ordered_i) != triangleNeighbours_j.end();

        if (!covered) {  // Worst case: find triangles (x, i, j) with x<i<j. Recursive search in triangle neighbours.
            bool neighbourhoodOf_i_isSmaller = triangleNeighbours_i.size() < triangleNeighbours_j.size();
            Index otherVertex = neighbourhoodOf_i_isSmaller ? ordered_j : ordered_i;

            for (auto& triangleNeighbours: neighbourhoodOf_i_isSmaller ? triangleNeighbours_i : triangleNeighbours_j) {
                if (triangleNeighbours.second.find(otherVertex) != triangleNeighbours.second.end()) {
                    covered = true;
                    break;
                }
            }
        }
    }

    return covered;
}

bool TriangleList::isPairCoveredExluding(const Index& i, const Index& j, const Triplet& triplet) const {
    size_t ordered_i = i<j? i : j;
    size_t ordered_j = i<j? j : i;

    auto orderedTriplet = triplet.getOrdered();

    std::list<Index> triplet_list {orderedTriplet.i, orderedTriplet.j, orderedTriplet.k};
    triplet_list.remove(i);
    triplet_list.remove(j);
    bool tripletCanCoverPair = triplet_list.size() == 1;

    if (!tripletCanCoverPair)
        return isPairCovered(i, j);


    bool covered = false;
    size_t vertexToIgnore = *triplet_list.begin();

    auto& triangleNeighbours_i = getTrianglesFrom(ordered_i);
    if (triangleNeighbours_i.find(ordered_j) != triangleNeighbours_i.end())
        covered = triangleNeighbours_i.at(ordered_j).size() > 1
                    || *triangleNeighbours_i.at(ordered_j).begin() != vertexToIgnore;

    auto& triangleNeighbours_j = getTrianglesFrom(ordered_j);
    if (!covered && triangleNeighbours_j.find(ordered_i) != triangleNeighbours_j.end())
        covered = triangleNeighbours_j.at(ordered_i).size() > 1
                    || *triangleNeighbours_j.at(ordered_i).begin() != vertexToIgnore;


    if (!covered) {  // Worst case: find triangles (x, i, j) with x<i<j. Recursive search in triangle neighbours.
        bool neighbourhoodOf_i_isSmaller = triangleNeighbours_i.size() < triangleNeighbours_j.size();
        Index otherVertex = neighbourhoodOf_i_isSmaller ? ordered_j : ordered_i;

        for (auto& triangleNeighbours: neighbourhoodOf_i_isSmaller ? triangleNeighbours_i : triangleNeighbours_j) {
            if (triangleNeighbours.first != vertexToIgnore
                    && triangleNeighbours.second.find(otherVertex) != triangleNeighbours.second.end()) {
                covered = true;
                break;
            }
        }
    }
    return covered;
}

size_t TriangleList::getTriangleNumberWith(const Index& vertex) const {
    size_t adjacentTriangleNumber = 0;
    for (auto& triangleNeighbours: getTrianglesFrom(vertex))
        adjacentTriangleNumber += triangleNeighbours.second.size();

    return adjacentTriangleNumber;
}

Edge TriangleList::getNthTriangleOfVertex(const Index& vertex, const size_t& n) const {
    Edge neighbours {0, 0};
    size_t n_copy(n);

    size_t triangleNumberWith_ij;
    bool found = false;

    for (auto& triangleNeighbours: getTrianglesFrom(vertex)) {
        auto& j = triangleNeighbours.first;

        triangleNumberWith_ij = triangleNeighbours.second.size();
        if (n_copy < triangleNumberWith_ij) {
            for (auto& k: triangleNeighbours.second) {
                if (n_copy == 0) {
                    neighbours = {j, k};
                    found = true;
                    break;
                }
                n_copy--;
            }
            break;
        }
        else
            n_copy -= triangleNumberWith_ij;
    }
    if (!found) throw std::out_of_range("Could not find "+std::to_string(n)+"th triangle of "+std::to_string(vertex));

    return neighbours;
}


void TriangleList::writeToBinary(const string &fileName) const{
    ofstream fileStream(fileName.c_str(), ios::out | ios::binary);
    if (!fileStream.is_open()) throw runtime_error("The file \""+fileName+"\" could not be open to save the triangle list");

    // Header
    // First 64 bits contain the size
    fileStream.write((char*) &size, sizeof(size_t));

    // Sequence of length "size" of 64 bits containing the length of every list of triangles
    size_t tmpListLength;
    for (size_t i=0; i<size; i++){
        tmpListLength = getTriangleNumberWith(i);
        fileStream.write((char*) &tmpListLength, sizeof(size_t));
    }

    // Sequence of length 2*triangleNumber of 64 bits with all the indices
    for (size_t i=0; i<size; i++) {
        for (auto& triangleNeighbours: getTrianglesFrom(i)) {
            auto& j=triangleNeighbours.first;

            for (auto& k: triangleNeighbours.second) {
                fileStream.write((char*) &j, sizeof(Index));
                fileStream.write((char*) &k, sizeof(Index));
            }
        }
    }

    fileStream.close();
}

TriangleList TriangleList::loadFromBinary(const string &fileName){
    ifstream fileStream(fileName.c_str(), ios::in | ios::binary);
    if (!fileStream.is_open()) throw runtime_error("The file could not be open to save the triangle list");

    // Header
    // First 64 bits contain the size
    size_t size;
    fileStream.read((char*) &size, sizeof(size_t));
    TriangleList returnedObject(size);

    // Sequence of length "size" containing the length in 64 bits of every list of triangles
    size_t listLengths[size];
    fileStream.read((char*) &listLengths, size*sizeof(size_t));

    // Sequence of length 2*triangleNumber of 64 bits with all the indices
    Index triplet_i = 0;
    Index triplet_jk[2];
    size_t listIndex = 0;
    long long unsigned int elementsAdded = 0;

    while ( fileStream.read((char*) &triplet_jk, 2*sizeof(Index)) ){
        while ( listIndex >= listLengths[triplet_i] ) {
            listIndex = 0;
            triplet_i++;
        }
        returnedObject.addTriangle({triplet_i, triplet_jk[0], triplet_jk[1]});
        elementsAdded++;
        listIndex++;
    }
    returnedObject.triangleNumber = elementsAdded/3;

    fileStream.close();
    return returnedObject;
}

} //namespace GRIT
