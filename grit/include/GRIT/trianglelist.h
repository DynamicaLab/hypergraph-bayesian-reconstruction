#ifndef GRIT_TRIANGLELIST_H
#define GRIT_TRIANGLELIST_H


#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <GRIT/utility.h>


namespace GRIT {


typedef size_t Index;
typedef std::unordered_map<Index, std::set<Index>> AdjacentTriangles;


struct Triplet {
    Index i;
    Index j;
    Index k;
    bool operator==(const Triplet& other) {
        return (i == other.i) && (j == other.j) && (k == other.k);
    }
    bool operator!=(const Triplet& other) {
        return !(*this == other);
    }
    Triplet getOrdered() const {
        std::vector<size_t> orderedList {i, j, k};
        std::sort(orderedList.begin(), orderedList.end());

        auto it = orderedList.begin();
        Triplet orderedTriplet;
        orderedTriplet.i = *it++;
        orderedTriplet.j = *it++;
        orderedTriplet.k = *it;
        return orderedTriplet;
    }
};

class TriangleList {
    public:
        explicit TriangleList(size_t size);
        size_t getSize() const { return size; };
        void resize(size_t new_size);
        size_t getTriangleNumber() const { return triangleNumber; };
        size_t getMaximumTriangleNumber() const { return nchoose3(size); }

        bool addTriangle(const Triplet&);
        bool removeTriangle(const Triplet&);
        bool isTriangle(const Triplet&) const;
        bool isPairCovered(const Index& i, const Index& j) const;
        bool isPairCoveredExluding(const Index& i, const Index& j, const Triplet&) const;

        const AdjacentTriangles& getTrianglesFrom(Index vertex) const{ return triangles[vertex]; };
        Edge getNthTriangleOfVertex(const Index& vertex, const size_t& n) const;
        size_t getTriangleNumberWith(const Index& vertex) const;
        const std::vector<AdjacentTriangles>& getTriangles() { return triangles; }


        void writeToBinary(const std::string& fileName) const;
        static TriangleList loadFromBinary(const std::string& fileName);

    protected:
        size_t size;
        size_t triangleNumber = 0;
        std::vector<AdjacentTriangles> triangles;
};

} //namespace GRIT

#endif
