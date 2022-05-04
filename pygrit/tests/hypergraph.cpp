#include <gtest/gtest.h>
#include <fstream>

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"


using namespace std;
using namespace GRIT;



TEST(Hypergraph, addMultiedge_validMultiedges_correctAdjacency) {
    Hypergraph hypergraph(4);
    EXPECT_TRUE(hypergraph.addMultiedge(0, 2, 1));
    EXPECT_TRUE(hypergraph.addMultiedge(0, 1, 3));
    EXPECT_TRUE(hypergraph.addEdge     (1, 0));
    EXPECT_TRUE(hypergraph.addMultiedge(2, 0, 2));

    EXPECT_EQ(hypergraph.getEdgesFrom(0), AdjacentEdges({ {2, 3}, {1, 4} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(1), AdjacentEdges({ {0, 4} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(2), AdjacentEdges({ {0, 3} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(3), AdjacentEdges({}));
    EXPECT_EQ(hypergraph.getEdgeNumber(), 2);
}


TEST(Hypergraph, removeEdge_existentMultiedge_multiplicityDecrement) {
    Hypergraph hypergraph(4);
    hypergraph.addMultiedge(0, 2, 2);
    hypergraph.addMultiedge(0, 1, 3);

    EXPECT_TRUE(hypergraph.removeEdge(0, 2));
    EXPECT_TRUE(hypergraph.removeEdge(0, 1));

    EXPECT_EQ(hypergraph.getEdgesFrom(0), AdjacentEdges({ {2, 1}, {1, 2} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(1), AdjacentEdges({ {0, 2} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(2), AdjacentEdges({ {0, 1} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(3), AdjacentEdges({}));
    EXPECT_EQ(hypergraph.getEdgeNumber(), 2);
}

TEST(Hypergraph, removeEdge_edgeOfMultiplicity1_edgeRemovedFromAdjacency) {
    Hypergraph hypergraph(4);
    hypergraph.addMultiedge(0, 2, 2);
    hypergraph.addEdge     (0, 1);

    EXPECT_TRUE(hypergraph.removeEdge(0, 1));

    EXPECT_EQ(hypergraph.getEdgesFrom(0), AdjacentEdges({ {2, 2} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(1), AdjacentEdges({ }));
    EXPECT_EQ(hypergraph.getEdgesFrom(2), AdjacentEdges({ {0, 2} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(3), AdjacentEdges({ }));
    EXPECT_EQ(hypergraph.getEdgeNumber(), 1);
}

TEST(Hypergraph, removeEdge_inexistentEdge_edgelistUnchanged) {
    Hypergraph hypergraph(4);
    hypergraph.addMultiedge(0, 2, 2);
    hypergraph.addMultiedge(0, 1, 3);

    EXPECT_FALSE(hypergraph.removeEdge(0, 3));
    EXPECT_FALSE(hypergraph.removeEdge(1, 2));

    EXPECT_EQ(hypergraph.getEdgesFrom(0), AdjacentEdges({ {2, 2}, {1, 3} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(1), AdjacentEdges({ {0, 3} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(2), AdjacentEdges({ {0, 2} }));
    EXPECT_EQ(hypergraph.getEdgesFrom(3), AdjacentEdges({}));
    EXPECT_EQ(hypergraph.getEdgeNumber(), 2);
}


TEST(Hypergraph, getEdgeMultiplicity_existentEdges_correctMultiplicity) {
    Hypergraph hypergraph(4);
    hypergraph.addMultiedge(0, 2, 2);
    hypergraph.addMultiedge(0, 1, 3);

    EXPECT_EQ(hypergraph.getEdgeMultiplicity(0, 2), 2);
    EXPECT_EQ(hypergraph.getEdgeMultiplicity(2, 0), 2);
    EXPECT_EQ(hypergraph.getEdgeMultiplicity(0, 1), 3);
    EXPECT_EQ(hypergraph.getEdgeMultiplicity(1, 0), 3);
}

TEST(Hypergraph, getEdgeMultiplicity_inexistentEdges_return0) {
    Hypergraph hypergraph(4);
    hypergraph.addMultiedge(0, 2, 2);
    hypergraph.addMultiedge(0, 1, 3);

    EXPECT_EQ(hypergraph.getEdgeMultiplicity(0, 3), 0);
    EXPECT_EQ(hypergraph.getEdgeMultiplicity(3, 0), 0);
    EXPECT_EQ(hypergraph.getEdgeMultiplicity(1, 2), 0);
}


TEST(Hypergraph, getHighestOrderHyperedgeWith_noHyperedge_return0) {
    Hypergraph hypergraph(5);

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeWith(0, 1), 0);
    hypergraph.addTriangle({0, 1, 2});
    hypergraph.addEdge(0, 1);

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeWith(0, 3), 0);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeWith(3, 0), 0);
}

TEST(Hypergraph, getHighestOrderHyperedgeWith_edgeNoTriangle_return1) {
    Hypergraph hypergraph(5);
    hypergraph.addTriangle({0, 1, 2});
    hypergraph.addEdge(0, 3);

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeWith(0, 3), 1);
}

TEST(Hypergraph, getHighestOrderHyperedgeWith_multiedgeNoTriangle_return1) {
    Hypergraph hypergraph(5);
    hypergraph.addTriangle({0, 1, 2});
    hypergraph.addMultiedge(0, 3, 2);

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeWith(0, 3), 1);
}

TEST(Hypergraph, getHighestOrderHyperedgeWith_triangle_return2) {
    Hypergraph hypergraph(5);
    hypergraph.addTriangle({0, 1, 3});

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeWith(0, 1), 2);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeWith(0, 3), 2);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeWith(1, 3), 2);
}

TEST(Hypergraph, getHighestOrderHyperedgeWith_triangleCoveringMultiedge_return2) {
    Hypergraph hypergraph(5);
    hypergraph.addTriangle({0, 1, 3});
    hypergraph.addMultiedge(0, 3, 2);

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeWith(0, 3), 2);
}


TEST(Hypergraph, getHighestOrderHyperedgeExcluding_noHyperedgeUnderTriangle_return0) {
    Hypergraph hypergraph(5);
    hypergraph.addTriangle({0, 1, 2});

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 1, {0, 1, 2}), 0);
    hypergraph.addEdge(0, 1);

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 2, {0, 1, 2}), 0);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(3, 0, {0, 1, 2}), 0);
}

TEST(Hypergraph, getHighestOrderHyperedgeExcluding_edgeUnderTriangle_return1) {
    Hypergraph hypergraph(5);
    hypergraph.addTriangle({0, 1, 2});

    hypergraph.addEdge(0, 1);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(1, 0, {0, 1, 2}), 1);
    hypergraph.addEdge(0, 2);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(2, 0, {0, 1, 2}), 1);
}

TEST(Hypergraph, getHighestOrderHyperedgeExcluding_multiedgeUnderTriangle_return1) {
    Hypergraph hypergraph(5);
    hypergraph.addTriangle({0, 1, 2});

    hypergraph.addMultiedge(0, 1, 2);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 1, {0, 1, 2}), 1);
    hypergraph.addMultiedge(0, 2, 2);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 2, {0, 1, 2}), 1);
}

TEST(Hypergraph, getHighestOrderHyperedgeExcluding_triangleUnderTriangle_return2) {
    Hypergraph hypergraph(5);
    hypergraph.addTriangle({0, 1, 3});
    hypergraph.addTriangle({0, 1, 4});
    hypergraph.addTriangle({1, 3, 4});

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 1, {0, 1, 2}), 2);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(3, 1, {0, 1, 2}), 2);
}

TEST(Hypergraph, getHighestOrderHyperedgeExcluding_triangleUnderTriangleCoveringMultiedge_return2) {
    Hypergraph hypergraph(5);
    hypergraph.addTriangle({0, 1, 3});
    hypergraph.addTriangle({0, 1, 4});
    hypergraph.addTriangle({1, 3, 4});
    hypergraph.addMultiedge(0, 1, 2);
    hypergraph.addMultiedge(1, 3, 2);

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 1, {0, 1, 2}), 2);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(1, 3, {0, 1, 2}), 2);
}


TEST(Hypergraph, getHighestOrderHyperedgeExcluding_sameEdge_return0) {
    Hypergraph hypergraph(5);
    hypergraph.addEdge(1, 0);

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 1, Edge{0, 1}), 0);
    hypergraph.addEdge(2, 0);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 2, Edge{2, 0}), 0);
}

TEST(Hypergraph, getHighestOrderHyperedgeWith_sameMultiedge_return0) {
    Hypergraph hypergraph(5);
    hypergraph.addMultiedge(1, 0, 2);

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 1, Edge{0, 1}), 0);
    hypergraph.addMultiedge(2, 0, 2);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 2, Edge{2, 0}), 0);
}

TEST(Hypergraph, getHighestOrderHyperedgeWith_edgeCoveredByTriangle_return2) {
    Hypergraph hypergraph(5);
    hypergraph.addTriangle({0, 1, 3});
    hypergraph.addEdge(0, 1);
    hypergraph.addEdge(0, 3);
    hypergraph.addEdge(1, 3);

    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 1, Edge{0, 1}), 2);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(0, 3, Edge{0, 3}), 2);
    EXPECT_EQ(hypergraph.getHighestOrderHyperedgeExcluding(1, 3, Edge{1, 3}), 2);
}


TEST(Hypergraph, writeToBinary_multiedges_correctMultiplicites) {
    Hypergraph hypergraph(5);
    EXPECT_TRUE(hypergraph.addMultiedge(0, 2, 1));
    EXPECT_TRUE(hypergraph.addMultiedge(0, 1, 3));
    EXPECT_TRUE(hypergraph.addEdge     (1, 0));
    EXPECT_TRUE(hypergraph.addMultiedge(2, 0, 2));


    hypergraph.writeEdgesToBinary("tmp_test.bin_edges");
    Hypergraph loadedHypergraph = Hypergraph::loadFromBinary("tmp_test.bin");

    EXPECT_EQ(loadedHypergraph.getEdgesFrom(0), AdjacentEdges({ {2, 3}, {1, 4} }));
    EXPECT_EQ(loadedHypergraph.getEdgesFrom(1), AdjacentEdges({ {0, 4} }));
    EXPECT_EQ(loadedHypergraph.getEdgesFrom(2), AdjacentEdges({ {0, 3} }));
    EXPECT_EQ(loadedHypergraph.getEdgesFrom(3), AdjacentEdges({}));
    EXPECT_EQ(loadedHypergraph.getEdgeNumber(), 2);
    remove("tmp_test.bin_edges");
}

TEST(Hypergraph, writeToBinary_triangles_correctTriangleAdjacency) {
    Hypergraph hypergraph(10);
    hypergraph.addTriangle({1, 3, 2});
    hypergraph.addTriangle({6, 1, 2});
    hypergraph.addTriangle({1, 8, 9});
    hypergraph.addTriangle({7, 8, 9});
    hypergraph.writeTrianglesToBinary("tmp_test.bin_triangles");

    Hypergraph loadedHypergraph = Hypergraph::loadFromBinary("tmp_test.bin");

    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(0), AdjacentTriangles{} );
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(1), AdjacentTriangles({ {2, {3, 6}}, {8, {9}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(2), AdjacentTriangles({ {1, {3, 6}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(3), AdjacentTriangles({ {1, {2}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(4), AdjacentTriangles{});
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(5), AdjacentTriangles{});
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(6), AdjacentTriangles({ {1, {2}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(7), AdjacentTriangles({ {8, {9}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(8), AdjacentTriangles({ {7, {9}}, {1, {9}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(9), AdjacentTriangles({ {7, {8}}, {1, {8}} }));
    EXPECT_EQ(loadedHypergraph.getTriangleNumber(), 4);
    remove("tmp_test.bin_triangles");
}

TEST(Hypergraph, writeToBinary_trianglesAndEdges_correctAdjacencies) {
    Hypergraph hypergraph(10);
    hypergraph.addMultiedge(0, 2, 1);
    hypergraph.addMultiedge(0, 1, 3);
    hypergraph.addEdge     (1, 0);
    hypergraph.addMultiedge(2, 0, 2);

    hypergraph.addTriangle({1, 3, 2});
    hypergraph.addTriangle({6, 1, 2});
    hypergraph.addTriangle({1, 8, 9});
    hypergraph.addTriangle({7, 8, 9});
    hypergraph.writeToBinary("tmp_test.bin");

    Hypergraph loadedHypergraph = Hypergraph::loadFromBinary("tmp_test.bin");

    EXPECT_EQ(loadedHypergraph.getEdgesFrom(0), AdjacentEdges({ {2, 3}, {1, 4} }));
    EXPECT_EQ(loadedHypergraph.getEdgesFrom(1), AdjacentEdges({ {0, 4} }));
    EXPECT_EQ(loadedHypergraph.getEdgesFrom(2), AdjacentEdges({ {0, 3} }));
    EXPECT_EQ(loadedHypergraph.getEdgesFrom(3), AdjacentEdges({}));
    EXPECT_EQ(loadedHypergraph.getEdgeNumber(), 2);
    remove("tmp_test.bin_edges");

    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(0), AdjacentTriangles{} );
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(1), AdjacentTriangles({ {2, {3, 6}}, {8, {9}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(2), AdjacentTriangles({ {1, {3, 6}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(3), AdjacentTriangles({ {1, {2}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(4), AdjacentTriangles{});
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(5), AdjacentTriangles{});
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(6), AdjacentTriangles({ {1, {2}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(7), AdjacentTriangles({ {8, {9}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(8), AdjacentTriangles({ {7, {9}}, {1, {9}} }));
    EXPECT_EQ(loadedHypergraph.getTrianglesFrom(9), AdjacentTriangles({ {7, {8}}, {1, {8}} }));
    EXPECT_EQ(loadedHypergraph.getTriangleNumber(), 4);
    remove("tmp_test.bin_triangles");
}
