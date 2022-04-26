#ifndef GRIT_HYPERGRAPH_H
#define GRIT_HYPERGRAPH_H


#include <vector>
#include <list>
#include <string>
#include "GRIT/trianglelist.h"

#include <GRIT/utility.h>


namespace GRIT {

using c_Index = const Index;
using AdjacentEdges = std::list<std::pair<size_t, size_t>>;


class Hypergraph : public TriangleList {
    public:
        explicit Hypergraph(size_t size);
        size_t getSize() const { return size; };
        void resize(size_t new_size);
        size_t getEdgeNumber() const { return edgeNumber; };
        size_t getMaximumEdgeNumber() const { return nchoose2(size); }

        bool addEdge(c_Index& vertex1, c_Index& vertex2);
        bool addMultiedge(c_Index& vertex1, c_Index& vertex2, size_t n);
        bool removeEdge(c_Index& vertex1, c_Index& vertex2);
        bool isEdge(c_Index& vertex1, c_Index& vertex2) const { return getEdgeMultiplicity(vertex1, vertex2) > 0; }
        size_t getEdgeMultiplicity(c_Index& vertex1, c_Index& vertex2) const;

        size_t getHighestOrderHyperedgeWith(c_Index& vertex1, c_Index& vertex2) const;
        size_t getHighestOrderHyperedgeExcluding(c_Index& vertex1, c_Index& vertex2, const Triplet& excludedTriplet) const;
        size_t getHighestOrderHyperedgeExcluding(c_Index& vertex1, c_Index& vertex2, const Edge& exludedEdge) const;

        const AdjacentEdges& getEdgesFrom(c_Index& vertex) const { return adjacencyLists[vertex]; }
        const std::list<Triplet> getFullTriangleList() const;

        void writeToBinary(const std::string& fileName) const;
        void writeTrianglesToBinary(const std::string& fileName) const { TriangleList::writeToBinary(fileName); };
        void writeEdgesToBinary(const std::string& fileName) const;

        static Hypergraph loadFromBinary(const std::string& filePrefix);

    private:
        size_t edgeNumber;
        std::vector<AdjacentEdges> adjacencyLists;

};

} //namespace GRIT

#endif
