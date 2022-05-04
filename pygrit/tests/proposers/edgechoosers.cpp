#include <gtest/gtest.h>

#include "GRIT/utility.h"
#include "GRIT/proposers/edge-choosers/uniform_edge_chooser.h"
#include "GRIT/proposers/edge-choosers/weighted_chooser.h"
#include "GRIT/proposers/edge-choosers/weighted_unique_chooser.h"
#include "GRIT/proposers/edge-choosers/weighted_two-layers_chooser.h"
#include "GRIT/proposers/edge-choosers/sep-weighted_unique_chooser.h"


using namespace std;
using namespace GRIT;


class HypergraphAndObservationsTestCase: public::testing::Test{
    public:
        Observations observations;
        Hypergraph hypergraph;

        HypergraphAndObservationsTestCase(): hypergraph(Hypergraph(4)) {}

        void SetUp(){
            hypergraph.addMultiedge(0, 1, 2);
            hypergraph.addEdge(0, 2);
            hypergraph.addEdge(1, 2);
            hypergraph.addTriangle({1, 2, 3});

            observations.resize(4, std::vector<size_t>(4, 0));
            size_t globalCount=0;
            for (auto i=3; i>=0; i--)
                for (auto j=i-1; j>=0; j--) {
                    observations[i][j] = globalCount;
                    observations[j][i] = globalCount;
                    globalCount++;
                }
        }
};

#define for_ij_in_observations\
    for (size_t i=0; i<observations.size(); i++)\
        for (size_t j=i+1; j<observations.size(); j++) {

#define EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS(expectedValue, method, move)\
    EXPECT_DOUBLE_EQ( expectedValue, chooser. method ({i, j}, move) );\
    EXPECT_DOUBLE_EQ( expectedValue, chooser. method ({j, i}, move) )

TEST_F(HypergraphAndObservationsTestCase, observationsWeightedChooser_expect_correctForwardProbabilities) {
    ObservationsWeightedEdgeChooser chooser(observations);

    double weightSum = 0;
    for_ij_in_observations
        weightSum += observations[i][j]+1;
    }

    for_ij_in_observations
        double weight = observations[i][j]+1;
        EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS(weight/weightSum, getForwardProbability, ADD);
    }
}

TEST_F(HypergraphAndObservationsTestCase, observationsWeightedChooser_expect_correctReverseProbabilities) {
    ObservationsWeightedEdgeChooser chooser(observations);

    double weightSum = 0;
    for_ij_in_observations
        weightSum += observations[i][j]+1;
    }

    for_ij_in_observations
        double weight = observations[i][j]+1;
        EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS(weight/weightSum, getForwardProbability, REMOVE);
    }
}

TEST_F(HypergraphAndObservationsTestCase, uniqueWeightedChooser_expect_correctForwardProbabilities) {
    ObservationsWeightedUniqueEdgeChooser chooser(observations, hypergraph);

    double weightSum = 0;
    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) == 0)
            weightSum += observations[i][j]+1;
    }

    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) == 0) {
            double weight = observations[i][j]+1;
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS(weight/weightSum, getForwardProbability, ADD);
        }
        /* Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this edge. Adding this check would cost performance
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS(0, getForwardProbability, ADD);
        }*/
    }
}

TEST_F(HypergraphAndObservationsTestCase, uniqueWeightedChooser_expect_correctReverseProbabilities) {
    ObservationsWeightedUniqueEdgeChooser chooser(observations, hypergraph);

    double weightSum = 0;
    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) == 0)
            weightSum += observations[i][j]+1;
    }

    for_ij_in_observations
        double weight = observations[i][j]+1;
        if (hypergraph.getEdgeMultiplicity(i,j) == 1) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/(weightSum+weight), getReverseProbability, REMOVE );
        }
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getReverseProbability, REMOVE );
        }
    }
}

TEST_F(HypergraphAndObservationsTestCase, uniqueWeightedChooser_expect_addEdgeToDistribution_when_edgeRemovedAndUpdating) {
    ObservationsWeightedUniqueEdgeChooser chooser(observations, hypergraph);


    chooser.updateProbabilities({0, 2}, REMOVE);
    hypergraph.removeEdge(0, 2);  // important because chooser probabilities rely on hypergraph

    double weightSum = 0;
    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) == 0)
            weightSum += observations[i][j]+1;
    }

    for_ij_in_observations
        double weight = observations[i][j]+1;
        if (hypergraph.getEdgeMultiplicity(i, j) == 0) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/weightSum, getForwardProbability, ADD);
        }
        /* Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this edge. Adding this check would cost performance
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getForwardProbability, ADD);
        } */

        if (hypergraph.getEdgeMultiplicity(i,j) == 1) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/(weightSum+weight), getReverseProbability, REMOVE );
        }
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getReverseProbability, REMOVE );
        }
    }
}

TEST_F(HypergraphAndObservationsTestCase, uniqueWeightedChooser_expect_removeEdgeFromDistribution_when_edgeAddedAndUpdating) {
    ObservationsWeightedUniqueEdgeChooser chooser(observations, hypergraph);


    chooser.updateProbabilities({0, 3}, ADD);
    hypergraph.addEdge(0, 3);  // important because chooser probabilities rely on hypergraph

    double weightSum = 0;
    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) == 0)
            weightSum += observations[i][j]+1;
    }

    for_ij_in_observations
        double weight = observations[i][j]+1;

        if (hypergraph.getEdgeMultiplicity(i, j) == 0) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/weightSum, getForwardProbability, ADD);
        }
        /* Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this edge. Adding this check would cost performance
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getForwardProbability, ADD);
        } */

        if (hypergraph.getEdgeMultiplicity(i,j) == 1) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/(weightSum+weight), getReverseProbability, REMOVE );
        }
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getReverseProbability, REMOVE );
        }
    }

}

TEST_F(HypergraphAndObservationsTestCase, nonZeroChooser_expect_correctForwardProbabilities) {
    UniformNonEdgeChooser chooser(hypergraph);

    size_t edgeNumber = hypergraph.getEdgeNumber();

    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) > 0) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 1./edgeNumber, getForwardProbability, REMOVE );
        }
        /* Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this edge. Adding this check would cost performance
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getForwardProbability, REMOVE);
        } */
    }
}

TEST_F(HypergraphAndObservationsTestCase, nonZeroChooser_expect_correctReverseProbabilities) {
    UniformNonEdgeChooser chooser(hypergraph);

    size_t edgeNumber = hypergraph.getEdgeNumber();

    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) == 0) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 1./(edgeNumber+1), getReverseProbability, ADD);
        }
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 1./edgeNumber, getReverseProbability, ADD);
        }
    }
}

TEST_F(HypergraphAndObservationsTestCase, nonZeroChooser_expect_removeEdgeFromDistribution_when_addEdgeAndUpdate) {
    UniformNonEdgeChooser chooser(hypergraph);

    chooser.updateProbabilities({0, 3}, ADD);
    hypergraph.addEdge(0, 3); // important because chooser probabilities rely on hypergraph
    size_t edgeNumber = hypergraph.getEdgeNumber();

    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) > 0) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 1./edgeNumber, getForwardProbability, REMOVE );
        }
        /* Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this edge. Adding this check would cost performance
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getForwardProbability, REMOVE);
        } */

        if (hypergraph.getEdgeMultiplicity(i, j) == 0) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 1./(edgeNumber+1), getReverseProbability, ADD );
        }
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 1./edgeNumber, getReverseProbability, ADD );
        }
    }

}

TEST_F(HypergraphAndObservationsTestCase, nonZeroChooser_expect_addEdgeToDistribution_when_removeEdgeAndUpdate) {
    UniformNonEdgeChooser chooser(hypergraph);

    chooser.updateProbabilities({0, 2}, REMOVE);
    hypergraph.removeEdge(0, 2); // important because chooser probabilities rely on hypergraph
    size_t edgeNumber = hypergraph.getEdgeNumber();

    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) > 0) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 1./edgeNumber, getForwardProbability, REMOVE );
        }
        /* Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this edge. Adding this check would cost performance
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getForwardProbability, REMOVE);
        } */

        if (hypergraph.getEdgeMultiplicity(i, j) == 0) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 1./(edgeNumber+1), getReverseProbability, ADD );
        }
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 1./edgeNumber, getReverseProbability, ADD );
        }
    }

}

TEST_F(HypergraphAndObservationsTestCase, twoLayersWeighted_expect_correctForwardProbabilities) {
    TwoLayersObservationsWeightedEdgeChooser chooser(observations, hypergraph);

    double weightSum = 0;
    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) < 2)
            weightSum += observations[i][j]+1;
    }

    for_ij_in_observations
        double weight = observations[i][j]+1;

        if (hypergraph.getEdgeMultiplicity(i, j) < 2) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/weightSum, getForwardProbability, ADD );
        }
        /* Expected behaviour, but this input is not used by the algorithm as the chooser will never propose this edge. Adding this check would cost performance
        else {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getForwardProbability, REMOVE);
        }*/
    }
}

TEST_F(HypergraphAndObservationsTestCase, twoLayersWeighted_expect_correctReverseProbabilities) {
    TwoLayersObservationsWeightedEdgeChooser chooser(observations, hypergraph);

    double weightSum = 0;
    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) < 2)
            weightSum += observations[i][j]+1;
    }

    for_ij_in_observations
        double weight = observations[i][j]+1;

        size_t edgeMultiplicity = hypergraph.getEdgeMultiplicity(i, j);
        if (edgeMultiplicity < 2) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/weightSum, getReverseProbability, REMOVE );
        }
        else if (edgeMultiplicity == 2) {
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/(weightSum+weight), getReverseProbability, REMOVE );
        }
    }
}

#define VERIFY_PROBS_TWOLAYERSWEIGHTED\
    for_ij_in_observations\
        double weight = observations[i][j]+1;\
        size_t edgeMultiplicity = hypergraph.getEdgeMultiplicity(i, j);\
\
        if (edgeMultiplicity < 2) {\
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/weightSum, getForwardProbability, ADD );\
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/weightSum, getReverseProbability, REMOVE );\
        }\
        else if (edgeMultiplicity == 2) {\
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( weight/(weightSum+weight), getReverseProbability, REMOVE );\
        }\
    }

TEST_F(HypergraphAndObservationsTestCase, twoLayersWeighted_expect_probabilitiesUnchanged_when_addingEdgeAtMultiplicity0) {
    TwoLayersObservationsWeightedEdgeChooser chooser(observations, hypergraph);

    chooser.updateProbabilities({0, 3}, ADD);

    double weightSum = 0;
    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) < 2)
            weightSum += observations[i][j]+1;
    }
    VERIFY_PROBS_TWOLAYERSWEIGHTED
}

TEST_F(HypergraphAndObservationsTestCase, twoLayersWeighted_expect_edgeRemovedFromDistribution_when_addingEdgeAtMultiplicity1) {
    TwoLayersObservationsWeightedEdgeChooser chooser(observations, hypergraph);

    chooser.updateProbabilities({0, 2}, ADD);
    hypergraph.addEdge(0, 2);

    double weightSum = 0;
    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) < 2)
            weightSum += observations[i][j]+1;
    }
    VERIFY_PROBS_TWOLAYERSWEIGHTED
}

TEST_F(HypergraphAndObservationsTestCase, twoLayersWeighted_expect_distributionUnchanged_when_removingEdgeAtMultiplicity1) {
    TwoLayersObservationsWeightedEdgeChooser chooser(observations, hypergraph);

    chooser.updateProbabilities({1, 2}, REMOVE);

    double weightSum = 0;
    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) < 2)
            weightSum += observations[i][j]+1;
    }
    VERIFY_PROBS_TWOLAYERSWEIGHTED
}

TEST_F(HypergraphAndObservationsTestCase, twoLayersWeighted_expect_edgeAddedToDistribution_when_removingEdgeAtMultiplicity2) {
    TwoLayersObservationsWeightedEdgeChooser chooser(observations, hypergraph);

    chooser.updateProbabilities({0, 1}, REMOVE);
    hypergraph.removeEdge(0, 1);

    double weightSum = 0;
    for_ij_in_observations
        if (hypergraph.getEdgeMultiplicity(i, j) < 2)
            weightSum += observations[i][j]+1;
    }


    VERIFY_PROBS_TWOLAYERSWEIGHTED
}

#define VERIFY_PROBS_SEP_UNIQUE_WEIGHTED\
        double weight = observations[i][j]+1;\
\
        if (!hypergraph.isEdge(i, j)) {\
            if (weight == 1){\
                EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( notObservedProb*weight/weightSum_noObservations, getForwardProbability, ADD );\
            }\
            else {\
                EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( (1-notObservedProb)*weight/weightSum_observations, getForwardProbability, ADD );\
            }\
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getReverseProbability, REMOVE );\
        }\
        else {\
            if (weight == 1){\
                EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( notObservedProb*weight/(weightSum_noObservations+weight), getReverseProbability, REMOVE );\
            }\
            else {\
                EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( (1-notObservedProb)*weight/(weightSum_observations+weight), getReverseProbability, REMOVE );\
            }\
        }

TEST_F(HypergraphAndObservationsTestCase, sepUniqueWeighted_expect_correctProbabilities_when_constructed) {
    double notObservedProb = .1;
    SeparatedWeightedUniqueEdgeChooser chooser(observations, hypergraph, notObservedProb);

    double weightSum_noObservations = 0;
    double weightSum_observations = 0;
    for_ij_in_observations
        if (!hypergraph.isEdge(i, j)) {
            if (observations[i][j]==0)
                weightSum_noObservations += 1;
            else
                weightSum_observations += observations[i][j]+1;
        }
    }

    for_ij_in_observations\
        if (i==1 && j==0 || i==0 && j==1) { // Edge case because edge (0,1) has multiplicity 2
            EXPECT_DOUBLE_EQ_BOTH_DIRECTIONS( 0, getReverseProbability, REMOVE );
        }
        else {
            VERIFY_PROBS_SEP_UNIQUE_WEIGHTED
        }
    }
}
