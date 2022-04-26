#include <gtest/gtest.h>

#include "GRIT/utility.h"
#include "GRIT/trianglelist.h"


using namespace std;
using namespace GRIT;


#define EXPECT_TRUE_ALL_PERMUTATIONS(func, i, j, k)\
    EXPECT_TRUE( func ({i, j, k}) );\
    EXPECT_TRUE( func ({i, k, j}) );\
    EXPECT_TRUE( func ({j, i, k}) );\
    EXPECT_TRUE( func ({j, k, i}) );\
    EXPECT_TRUE( func ({k, i, j}) );\
    EXPECT_TRUE( func ({k, j, i}) )

#define EXPECT_FALSE_ALL_PERMUTATIONS(func, i, j, k)\
    EXPECT_FALSE( func ({i, j, k}) );\
    EXPECT_FALSE( func ({i, k, j}) );\
    EXPECT_FALSE( func ({j, i, k}) );\
    EXPECT_FALSE( func ({j, k, i}) );\
    EXPECT_FALSE( func ({k, i, j}) );\
    EXPECT_FALSE( func ({k, j, i}) )


TEST(TriangleList, addTriangle_validTriangles_everyTriangleAdded) {
    TriangleList triangleList(4);

    EXPECT_TRUE(triangleList.addTriangle({0, 1, 2}));
    EXPECT_TRUE(triangleList.addTriangle({0, 1, 3}));
    EXPECT_TRUE(triangleList.addTriangle({0, 2, 3}));

    EXPECT_EQ(triangleList.getTrianglesFrom(0), AdjacentTriangles({ {1, {2, 3}}, {2, {3}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(1), AdjacentTriangles({ {0, {2, 3}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(2), AdjacentTriangles({ {0, {1, 3}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(3), AdjacentTriangles({ {0, {1, 2}} }));
    EXPECT_EQ(triangleList.getTriangleNumber(), 3);
}

TEST(TriangleList, addTriangle_repeatedTriangle_triangleNotAdded) {
    TriangleList triangleList(4);
    EXPECT_TRUE(triangleList.addTriangle({0, 1, 2}));
    EXPECT_TRUE(triangleList.addTriangle({0, 1, 3}));
    EXPECT_TRUE(triangleList.addTriangle({0, 2, 3}));

    EXPECT_FALSE(triangleList.addTriangle({0, 1, 3}));
    EXPECT_FALSE(triangleList.addTriangle({0, 1, 2}));

    EXPECT_EQ(triangleList.getTrianglesFrom(0), AdjacentTriangles({ {1, {2, 3}}, {2, {3}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(1), AdjacentTriangles({ {0, {2, 3}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(2), AdjacentTriangles({ {0, {1, 3}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(3), AdjacentTriangles({ {0, {1, 2}} }));
    EXPECT_EQ(triangleList.getTriangleNumber(), 3);
}


TEST(TriangleList, removeTriangle_existentTriangle_triangleRemoved) {
    TriangleList triangleList(4);
    EXPECT_TRUE(triangleList.addTriangle   ({0, 1, 2}));
    EXPECT_TRUE(triangleList.addTriangle   ({0, 1, 3}));
    EXPECT_TRUE(triangleList.addTriangle   ({0, 2, 3}));

    EXPECT_TRUE(triangleList.removeTriangle({0, 1, 3}));

    EXPECT_EQ(triangleList.getTrianglesFrom(0), AdjacentTriangles({ {1, {2}}, {2, {3}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(1), AdjacentTriangles({ {0, {2}}    }));
    EXPECT_EQ(triangleList.getTrianglesFrom(2), AdjacentTriangles({ {0, {1, 3}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(3), AdjacentTriangles({ {0, {2}}    }));
    EXPECT_EQ(triangleList.getTriangleNumber(), 2);
}

TEST(TriangleList, removeTriangle_lastTriangleOfPair_trianglesFromPairRemoved) {
    TriangleList triangleList(4);
    EXPECT_TRUE(triangleList.addTriangle   ({0, 1, 2}));
    EXPECT_TRUE(triangleList.addTriangle   ({0, 2, 3}));

    EXPECT_TRUE(triangleList.removeTriangle({0, 2, 3}));

    EXPECT_EQ(triangleList.getTrianglesFrom(0), AdjacentTriangles({ {1, {2}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(1), AdjacentTriangles({ {0, {2}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(2), AdjacentTriangles({ {0, {1}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(3), AdjacentTriangles({}));
    EXPECT_EQ(triangleList.getTriangleNumber(), 1);
}

TEST(TriangleList, removeTriangle_inexistentTriangle_doNothing) {
    TriangleList triangleList(4);

    EXPECT_FALSE(triangleList.removeTriangle({0, 1, 3}));
    EXPECT_TRUE (triangleList.addTriangle   ({0, 1, 2}));
    EXPECT_FALSE(triangleList.removeTriangle({0, 1, 3}));


    EXPECT_EQ(triangleList.getTrianglesFrom(0), AdjacentTriangles({ {1, {2}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(1), AdjacentTriangles({ {0, {2}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(2), AdjacentTriangles({ {0, {1}} }));
    EXPECT_EQ(triangleList.getTrianglesFrom(3), AdjacentTriangles({}));
    EXPECT_EQ(triangleList.getTriangleNumber(), 1);
}


TEST(TriangleList, isTriangle_existentTriangle_returnTrue) {
    TriangleList triangleList(4);
    EXPECT_TRUE (triangleList.addTriangle   ({0, 1, 2}));
    EXPECT_TRUE (triangleList.addTriangle   ({0, 1, 3}));
    EXPECT_TRUE (triangleList.addTriangle   ({0, 2, 3}));

    EXPECT_TRUE_ALL_PERMUTATIONS(triangleList.isTriangle, 0, 1, 2);
    EXPECT_TRUE_ALL_PERMUTATIONS(triangleList.isTriangle, 0, 1, 3);
    EXPECT_TRUE_ALL_PERMUTATIONS(triangleList.isTriangle, 0, 2, 3);
}

TEST(TriangleList, isTriangle_inexistentTriangle_returnTrue) {
    TriangleList triangleList(5);

    EXPECT_FALSE_ALL_PERMUTATIONS(triangleList.isTriangle, 0, 1, 2);
    EXPECT_TRUE (triangleList.addTriangle   ({0, 1, 2}));
    EXPECT_TRUE (triangleList.addTriangle   ({0, 1, 3}));
    EXPECT_TRUE (triangleList.addTriangle   ({0, 2, 3}));

    EXPECT_FALSE_ALL_PERMUTATIONS(triangleList.isTriangle, 0, 1, 4);
    EXPECT_FALSE_ALL_PERMUTATIONS(triangleList.isTriangle, 0, 2, 4);
    EXPECT_FALSE_ALL_PERMUTATIONS(triangleList.isTriangle, 1, 2, 3);
}


TEST(TriangleList, isPairCovered_pairNotCovered_returnFalse) {
    TriangleList triangleList(4);

    EXPECT_FALSE(triangleList.isPairCovered(0, 2));
    triangleList.addTriangle({1, 2, 3});

    EXPECT_FALSE(triangleList.isPairCovered(0, 1));
}

TEST(TriangleList, isPairCovered_pairCovered_returnTrue) {
    TriangleList triangleList(4);
    triangleList.addTriangle({0, 1, 2});
    triangleList.addTriangle({0, 1, 3});

    EXPECT_TRUE(triangleList.isPairCovered(0, 1));
    EXPECT_TRUE(triangleList.isPairCovered(0, 2));
    EXPECT_TRUE(triangleList.isPairCovered(0, 3));
    EXPECT_TRUE(triangleList.isPairCovered(1, 2));
}


TEST(TriangleList, isPairCoveredExcluding_onlyOneTriangle_returnFalse) {
    TriangleList triangleList(4);
    triangleList.addTriangle({0, 1, 2});

    EXPECT_FALSE(triangleList.isPairCoveredExluding(0, 1, {0, 1, 2}));
    EXPECT_FALSE(triangleList.isPairCoveredExluding(0, 2, {0, 1, 2}));
    EXPECT_FALSE(triangleList.isPairCoveredExluding(1, 2, {0, 1, 2}));
}

TEST(TriangleList, isPairCoveredExcluding_triangleUnderTriangle_returnTrue) {
    TriangleList triangleList(4);
    triangleList.addTriangle({0, 1, 2});
    triangleList.addTriangle({0, 1, 3});
    triangleList.addTriangle({1, 2, 3});

    EXPECT_TRUE(triangleList.isPairCoveredExluding(0, 1, {0, 1, 2}));
    EXPECT_TRUE(triangleList.isPairCoveredExluding(1, 2, {0, 1, 2}));
}


TEST(TriangleList, getTriangleNumberWith_complexTriangleList_returnLength) {
    TriangleList triangleList(7);

    triangleList.addTriangle({0, 1, 2});
    triangleList.addTriangle({0, 1, 3});
    triangleList.addTriangle({0, 2, 3});
    triangleList.addTriangle({0, 3, 4});
    triangleList.addTriangle({0, 3, 5});

    EXPECT_EQ(triangleList.getTriangleNumberWith(0), 5);
    EXPECT_EQ(triangleList.getTriangleNumberWith(1), 2);
    EXPECT_EQ(triangleList.getTriangleNumberWith(2), 2);
    EXPECT_EQ(triangleList.getTriangleNumberWith(3), 4);
    EXPECT_EQ(triangleList.getTriangleNumberWith(4), 1);
    EXPECT_EQ(triangleList.getTriangleNumberWith(5), 1);
    EXPECT_EQ(triangleList.getTriangleNumberWith(6), 0);
}


TEST(TriangleList, getNthTriangleOfVertex_complexTriangleList_correctTriangle) {
    TriangleList triangleList(7);

    triangleList.addTriangle({0, 1, 2});
    triangleList.addTriangle({0, 1, 3});
    triangleList.addTriangle({0, 2, 3});
    triangleList.addTriangle({0, 3, 4});
    triangleList.addTriangle({0, 3, 5});

    using Edges = std::vector<std::pair<size_t, size_t>>;
    Edges expectedEdges = {{1, 2}, {1, 3}, {2, 3}, {3, 4}, {3, 5} };
    Edges actualEdges; 

    for (size_t i=0; i<5; i++)
        actualEdges.push_back(triangleList.getNthTriangleOfVertex(0, i));

    std::sort(expectedEdges.begin(), expectedEdges.end());
    std::sort(actualEdges.begin(), actualEdges.end());
    EXPECT_EQ(expectedEdges, actualEdges);

    //EXPECT_EQ(triangleList.getNthTriangleOfVertex(0, 0), Edge(Edge{1, 2}));
    //EXPECT_EQ(triangleList.getNthTriangleOfVertex(0, 1), Edge(Edge{1, 3}));
    //EXPECT_EQ(triangleList.getNthTriangleOfVertex(0, 2), Edge(Edge{2, 3}));
    //EXPECT_EQ(triangleList.getNthTriangleOfVertex(0, 3), Edge(Edge{3, 4}));
    //EXPECT_EQ(triangleList.getNthTriangleOfVertex(0, 4), Edge(Edge{3, 5}));
}


TEST(TriangleList, loadAndWriteBinary_empty_triangleListEmpty) {
    TriangleList triangleList(4);

    triangleList.writeToBinary("testBinary.bin");

    TriangleList loadedTriangleList(5);
    loadedTriangleList.loadFromBinary("testBinary.bin");
    remove("testBinary.bin");

    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(0), AdjacentTriangles{} );
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(1), AdjacentTriangles{} );
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(2), AdjacentTriangles{} );
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(3), AdjacentTriangles{} );
    EXPECT_EQ(loadedTriangleList.getTriangleNumber(), 0);
}

TEST(TriangleList, loadAndWriteBinary_trianglesAndEmptyLists_informationRetreived) {
    TriangleList triangleList(10);

    triangleList.addTriangle({1, 3, 2});
    triangleList.addTriangle({6, 1, 2});
    triangleList.addTriangle({1, 8, 9});
    triangleList.addTriangle({7, 8, 9});
    triangleList.writeToBinary("testBinary.bin");

    TriangleList loadedTriangleList = TriangleList::loadFromBinary("testBinary.bin");
    remove("testBinary.bin");

    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(0), AdjacentTriangles{} );
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(1), AdjacentTriangles({ {2, {3, 6}}, {8, {9}} }));
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(2), AdjacentTriangles({ {1, {3, 6}} }));
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(3), AdjacentTriangles({ {1, {2}} }));
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(4), AdjacentTriangles{});
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(5), AdjacentTriangles{});
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(6), AdjacentTriangles({ {1, {2}} }));
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(7), AdjacentTriangles({ {8, {9}} }));
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(8), AdjacentTriangles({ {7, {9}}, {1, {9}} }));
    EXPECT_EQ(loadedTriangleList.getTrianglesFrom(9), AdjacentTriangles({ {7, {8}}, {1, {8}} }));
    EXPECT_EQ(loadedTriangleList.getTriangleNumber(), 4);
}
