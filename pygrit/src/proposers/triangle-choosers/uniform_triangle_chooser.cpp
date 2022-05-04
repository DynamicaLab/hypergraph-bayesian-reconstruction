#include <stdexcept>
#include <random>

#include "GRIT/proposers/triangle-choosers/uniform_triangle_chooser.h"


namespace GRIT {

UniformTriangleChooser::UniformTriangleChooser(const Hypergraph& hypergraph):
    samplableSet(1, 1000000), hypergraph(hypergraph) {
    buildSamplableSetFromGraph();
}


void UniformTriangleChooser::buildSamplableSetFromGraph(){
    samplableSet.clear();

    size_t adjacentTriangleNumber;
    for (size_t i=0; i<hypergraph.getSize(); i++) {

        adjacentTriangleNumber = hypergraph.getTriangleNumberWith(i);

        if (adjacentTriangleNumber > 0)
            samplableSet.insert(i, adjacentTriangleNumber);
    }
}

void UniformTriangleChooser::recomputeDistribution() {
    buildSamplableSetFromGraph();
}


Triplet UniformTriangleChooser::choose(){
    if (hypergraph.getTriangleNumber() == 0) throw std::logic_error("There are no triangle to sample");

    size_t firstVertex = samplableSet.sample_ext_RNG<std::mt19937>(generator).first;
    std::pair<size_t, size_t> neighbours = drawTriangleUniformelyFromVertex(firstVertex);

    if (hypergraph.getTrianglesFrom(firstVertex).size() == 0 ) throw std::logic_error("First vertex has no triangle");
    if (hypergraph.getTrianglesFrom(neighbours.first).size() == 0 ) throw std::logic_error("Second vertex has no triangle");
    if (hypergraph.getTrianglesFrom(neighbours.second).size() == 0 ) throw std::logic_error("Third vertex has no triangle");

    return Triplet({firstVertex, neighbours.first, neighbours.second});
}

std::pair<size_t, size_t> UniformTriangleChooser::drawTriangleUniformelyFromVertex(size_t vertex) const{
    size_t adjacentTriangleNumber = samplableSet.get_weight(vertex);

    size_t chosenIndex = std::uniform_int_distribution<size_t>(0, adjacentTriangleNumber-1)(generator);
    return hypergraph.getNthTriangleOfVertex(vertex, chosenIndex);
}

double UniformTriangleChooser::getForwardProbability(const Triplet &, const AddRemoveMove &) const{
    return 1.0/ (double) hypergraph.getTriangleNumber();
}

double UniformTriangleChooser::getReverseProbability(const Triplet& triplet, const AddRemoveMove &) const{
    if (hypergraph.isTriangle(triplet))  // Triangles always exist only once
        return 1.0/ (double) (hypergraph.getTriangleNumber());
    return 1.0/ (double) (hypergraph.getTriangleNumber()+1);
}

void UniformTriangleChooser::updateProbabilities(const Triplet &triplet, const AddRemoveMove &move) {
    bool isTriangle = hypergraph.isTriangle(triplet);
    if (move == ADD && isTriangle || move == REMOVE && !isTriangle)
        return;
    updateWeightOfIndex(triplet.i, move);
    updateWeightOfIndex(triplet.j, move);
    updateWeightOfIndex(triplet.k, move);
}

void UniformTriangleChooser::updateWeightOfIndex(const size_t& index, const AddRemoveMove& move) {
    size_t neighbouringTrianglesNumber = 0;
    if (samplableSet.count(index))
        neighbouringTrianglesNumber = samplableSet.get_weight(index);

    if (move == ADD) {
        neighbouringTrianglesNumber++;
        if (neighbouringTrianglesNumber == 1)
            samplableSet.insert(index, neighbouringTrianglesNumber);
        else
            samplableSet.set_weight(index, neighbouringTrianglesNumber);
    }
    else {
        if (neighbouringTrianglesNumber == 0)
            throw std::runtime_error("Uniform triangle chooser: removing a triangle where there is not");
        neighbouringTrianglesNumber--;
        if (neighbouringTrianglesNumber == 0)
            samplableSet.erase(index);
        else
            samplableSet.set_weight(index, neighbouringTrianglesNumber);
    }
}

} //namespace GRIT
