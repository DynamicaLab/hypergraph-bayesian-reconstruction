#include <gtest/gtest.h>

#include "GRIT/utility.h"
#include "GRIT/proposers/triangle-choosers/observations_by_pairs_chooser.h"
#include "GRIT/proposers/triangle-choosers/triplet_three_steps.h"
#include "GRIT/proposers/triangle-choosers/uniform_triangle_chooser.h"


using namespace std;
using namespace GRIT;


class HypergraphAndObservationsTestCase: public::testing::Test{
    public:
        Observations observations;
        Hypergraph hypergraph;
        size_t triangleNumber = 2;

        HypergraphAndObservationsTestCase(): hypergraph(Hypergraph(4)) {}

        void SetUp(){
            hypergraph.addMultiedge(0, 1, 2);
            hypergraph.addEdge(0, 2);
            hypergraph.addEdge(1, 2);
            hypergraph.addTriangle({1, 2, 3});
            hypergraph.addTriangle({0, 2, 3});

            observations.resize(4, std::vector<size_t>(4, 0));
            size_t globalCount=0;
            for (auto i=0; i<4; i++)
                for (auto j=i+1; j<4; j++) {
                    observations[i][j] = globalCount;
                    observations[j][i] = globalCount;
                    globalCount++;
                }
        }
};

#define for_ijk_in_observations\
    for (size_t i=0; i<observations.size(); i++)\
        for (size_t j=i+1; j<observations.size(); j++)\
            for (size_t k=j+1; k<observations.size(); k++) {

#define EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS(expectedValue, method, move)\
    EXPECT_DOUBLE_EQ( expectedValue, chooser. method ({i, j, k}, move) );\
    EXPECT_DOUBLE_EQ( expectedValue, chooser. method ({i, k, j}, move) );\
    EXPECT_DOUBLE_EQ( expectedValue, chooser. method ({j, i, k}, move) );\
    EXPECT_DOUBLE_EQ( expectedValue, chooser. method ({j, k, i}, move) );\
    EXPECT_DOUBLE_EQ( expectedValue, chooser. method ({k, i, j}, move) );\
    EXPECT_DOUBLE_EQ( expectedValue, chooser. method ({k, j, i}, move) )


static inline double getWeight(size_t obs) { return obs+1; }

static tuple<vector<double>, vector<vector<double>>, double> getPairwiseObservationWeights(const Observations& observations) {

    const size_t& n = observations.size();
    vector<double> vertexWeights(n, 0);
    vector<vector<double>> pairwiseWeights(n, vector<double>(n, 0) );
    double weightSum = 0;

    double weight;
    for (size_t i=0; i<n; i++) {
        for (size_t j=0; j<n; j++) {
            if (i == j) continue;

            weight = getWeight(observations[i][j]);
            vertexWeights[i] += weight;
            pairwiseWeights[i][j] = weight;
            weightSum += weight;
        }
    }
    return {vertexWeights, pairwiseWeights, weightSum};
}

TEST_F(HypergraphAndObservationsTestCase, observationsByPairs_expect_correctForwardProbabilities) {
    ObservationsPairwiseTriangleChooser chooser(observations);

    auto[v, w, sum] = getPairwiseObservationWeights(observations);
    for_ijk_in_observations
        EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( ( w[i][j]*(w[k][i]/v[i] + w[k][j]/v[j]) + w[i][k]*(w[j][i]/v[i] + w[j][k]/v[k]) + w[j][k]*(w[i][j]/v[j] + w[i][k]/v[k]) ) / sum,
                getForwardProbability, AddRemoveMove::ADD );
}
}

TEST_F(HypergraphAndObservationsTestCase, observationsByPairs_expect_correctReverseProbabilities) {
    ObservationsPairwiseTriangleChooser chooser(observations);

    auto[v, w, sum] = getPairwiseObservationWeights(observations);
    for_ijk_in_observations
        EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( ( w[i][j]*(w[k][i]/v[i] + w[k][j]/v[j]) + w[i][k]*(w[j][i]/v[i] + w[j][k]/v[k]) + w[j][k]*(w[i][j]/v[j] + w[i][k]/v[k]) ) / sum,
                                                getReverseProbability, AddRemoveMove::REMOVE );
    }
}

TEST_F(HypergraphAndObservationsTestCase, uniformRemovalChooser_expect_correctForwardProbabilities) {
    UniformTriangleChooser chooser(hypergraph);
    auto triangles = hypergraph.getFullTriangleList();

    for_ijk_in_observations
        if (std::find(triangles.begin(), triangles.end(), Triplet({i, j, k}) ) != triangles.end()) {
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./triangleNumber, getForwardProbability, AddRemoveMove::REMOVE );
        }
        // Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this triangle. Adding this check would cost performance
        //else {
            //EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 0, getForwardProbability, AddRemoveMove::REMOVE );
        //}
    }
}

TEST_F(HypergraphAndObservationsTestCase, uniformRemovalChooser_expect_correctReverseProbabilities) {
    UniformTriangleChooser chooser(hypergraph);
    auto triangles = hypergraph.getFullTriangleList();

    for_ijk_in_observations
        if (std::find(triangles.begin(), triangles.end(), Triplet({i, j, k}) ) == triangles.end()) {
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./(triangleNumber+1), getReverseProbability, AddRemoveMove::ADD );
        }
        else {
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./triangleNumber, getReverseProbability, AddRemoveMove::ADD );
        }
    }
}

TEST_F(HypergraphAndObservationsTestCase, uniformRemovalChooser_when_addNonExistentTriangle_expect_probabilitiesUpdated) {
    UniformTriangleChooser chooser(hypergraph);

    chooser.updateProbabilities({0, 1, 2}, AddRemoveMove::ADD);
    hypergraph.addTriangle({0, 1, 2});

    triangleNumber++;
    auto triangles = hypergraph.getFullTriangleList();

    for_ijk_in_observations
        if (std::find(triangles.begin(), triangles.end(), Triplet({i, j, k}) ) != triangles.end()) {
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./triangleNumber, getForwardProbability, AddRemoveMove::REMOVE );
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./triangleNumber, getReverseProbability, AddRemoveMove::ADD );
        }
        else {
            // Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this triangle. Adding this check would cost performance
            // EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 0, getForwardProbability, AddRemoveMove::REMOVE );
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./(triangleNumber+1), getReverseProbability, AddRemoveMove::ADD );
        }
    }
}

TEST_F(HypergraphAndObservationsTestCase, uniformRemovalChooser_when_addExistentTriangle_expect_probabilitiesUnchanged) {
    UniformTriangleChooser chooser(hypergraph);

    chooser.updateProbabilities({1, 2, 3}, AddRemoveMove::ADD);

    auto triangles = hypergraph.getFullTriangleList();

    for_ijk_in_observations
        if (std::find(triangles.begin(), triangles.end(), Triplet({i, j, k}) ) != triangles.end()) {
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./triangleNumber, getForwardProbability, AddRemoveMove::REMOVE );
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./triangleNumber, getReverseProbability, AddRemoveMove::ADD );
        }
        else {
            // Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this triangle. Adding this check would cost performance
            // EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 0, getForwardProbability, AddRemoveMove::REMOVE );
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./(triangleNumber+1), getReverseProbability, AddRemoveMove::ADD );
        }
    }
}

TEST_F(HypergraphAndObservationsTestCase, uniformRemovalChooser_when_removeExistentTriangle_expect_probabilitiesUpdated) {
    UniformTriangleChooser chooser(hypergraph);

    chooser.updateProbabilities({1, 2, 3}, AddRemoveMove::ADD);
    hypergraph.removeTriangle({1, 2, 3});

    triangleNumber--;
    auto triangles = hypergraph.getFullTriangleList();

    for_ijk_in_observations
        if (std::find(triangles.begin(), triangles.end(), Triplet({i, j, k}) ) != triangles.end()) {
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./triangleNumber, getForwardProbability, AddRemoveMove::REMOVE );
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./triangleNumber, getReverseProbability, AddRemoveMove::ADD );
        }
        else {
            // Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this triangle. Adding this check would cost performance
            // EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 0, getForwardProbability, AddRemoveMove::REMOVE );
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./(triangleNumber+1), getReverseProbability, AddRemoveMove::ADD );
        }
    }
}

TEST_F(HypergraphAndObservationsTestCase, uniformRemovalChooser_when_removeNonExistentTriangle_expect_probabilitiesUnchanged) {
    UniformTriangleChooser chooser(hypergraph);

    chooser.updateProbabilities({0, 1, 2}, AddRemoveMove::ADD);

    auto triangles = hypergraph.getFullTriangleList();

    for_ijk_in_observations
        if (std::find(triangles.begin(), triangles.end(), Triplet({i, j, k}) ) != triangles.end()) {
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./triangleNumber, getForwardProbability, AddRemoveMove::REMOVE );
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./triangleNumber, getReverseProbability, AddRemoveMove::ADD );
        }
        else {
            // Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this triangle. Adding this check would cost performance
            // EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 0, getForwardProbability, AddRemoveMove::REMOVE );
            EXPECT_DOUBLE_EQ_ALL_PERMUTATIONS( 1./(triangleNumber+1), getReverseProbability, AddRemoveMove::ADD );
        }
    }
}
