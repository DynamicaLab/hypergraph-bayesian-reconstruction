#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GRIT/utility.h"
#include "GRIT/hypergraph.h"
#include <list>
#include <unordered_set>


namespace py = pybind11;


template<typename T>
static bool areAllDifferent(const std::vector<T>& elements);
static inline size_t choose2(size_t n);
static size_t findSBMGroupFromVertex(const std::vector<double>& communitySizes, size_t index);
static void addEdgesWithProbability(GRIT::Hypergraph&, double p);


GRIT::Hypergraph generateIndependentLayeredEdges(size_t n, double q1, double q2) {
    GRIT::Hypergraph hypergraph(n);

    addEdgesWithProbability(hypergraph, q1);

    size_t i=0;
    size_t j=0;

    std::uniform_real_distribution<double> distribution(0, 1);
    double r;

    while (i<n) {
        r = distribution(GRIT::generator);
        j += 1+floor(log(1-r)/log(1-q2));
        while (j >= i && i < n) {
            j -= i;
            i++;
        }
        if (i<n) {
            if (hypergraph.isEdge(i, j))
                hypergraph.addEdge(i, j);
            else
                hypergraph.addMultiedge(i, j, 2);
        }
    }
    return hypergraph;
}

GRIT::Hypergraph generateIndependentTriangleHypergraph(size_t n, double p){
    GRIT::Hypergraph hypergraph(n);
    size_t i(0), j(1), k(2);
    size_t cumulativeI(0), cumulativeJ(0);
    long long idx = -1;

    std::uniform_real_distribution<double> distribution(0, 1);
    double r;
    size_t nchoose3 = n*(n-1)*(n-2)/6;

    while (true){
        r = distribution(GRIT::generator);
        idx += 1 + floor( log(1-r)/log(1-p) );

        if (idx >= nchoose3) break;

        while (idx-cumulativeI >= choose2(n-i-1)) {
            cumulativeJ = 0;
            cumulativeI += choose2(n-i-1);
            i++;
            j = i+1;
        }
        while (idx-cumulativeI-cumulativeJ >= n-j-1){
            cumulativeJ += n-j-1;
            j += 1;
        }
        k = j+1 + idx-cumulativeJ-cumulativeI;
        hypergraph.addTriangle({i, j, k});
    }
    return hypergraph;
}

GRIT::Hypergraph generateIndependentHyperedgesHypergraph(size_t n, double p, double q) {
    GRIT::Hypergraph hypergraph = generateIndependentTriangleHypergraph(n, p);
    addEdgesWithProbability(hypergraph, q);

    return hypergraph;
}

// Adapted from https://stackoverflow.com/questions/38993415/how-to-apply-the-intersection-between-two-lists-in-c
template<typename T>
static std::list<T> intersectionOf(const std::list<T>& a, const std::list<T>& b){
    std::list<T> rtn;
    std::multiset<T> st;
    std::for_each(a.begin(), a.end(), [&st](const T& k){ st.insert(k); });
    std::for_each(b.begin(), b.end(),
        [&st, &rtn](const T& k){
            auto iter = st.find(k);
            if(iter != st.end()){
                rtn.push_back(k);
                st.erase(iter);
            }
        }
    );
    return rtn;
}

GRIT::Hypergraph generateIndependentHyperedgeOnlyCycles(size_t size, size_t subgraphSize, double triangleProbability) {
    if (subgraphSize == 0)
        throw std::logic_error("Complete graphs must have a non zero size.");
    GRIT::Hypergraph hypergraph(size);
    std::bernoulli_distribution createTriangleDistribution(triangleProbability);

    // Create disjoint subgraphs
    for (size_t subgraph=0; subgraph<size/subgraphSize; subgraph++)
        for (size_t i=subgraph*subgraphSize; i<(subgraph+1)*subgraphSize; i++)
            for (size_t j=i; j<(subgraph+1)*subgraphSize; j++)
                hypergraph.addEdge(i, j);


    // Promote 3-cycles of edges to hyperedge with given probability
    for (size_t vertex1=0; vertex1<size; vertex1++) {
        auto& vertex1Neighbours = hypergraph.getEdgesFrom(vertex1);

        for (auto vertex2: vertex1Neighbours)
            if (vertex1 < vertex2.first)
                for (auto vertex3: intersectionOf(vertex1Neighbours, hypergraph.getEdgesFrom(vertex2.first)))
                    if (vertex2 < vertex3 && createTriangleDistribution(GRIT::generator))
                        hypergraph.addTriangle({vertex1, vertex2.first, vertex3.first});
    }
    return hypergraph;
}

GRIT::Hypergraph generateIndependentHyperedgeWithoutCycles(size_t n, double triangleProbability, double edgeProbability){
    GRIT::Hypergraph hypergraph = generateIndependentTriangleHypergraph(n, triangleProbability);

    size_t i=0;
    size_t j=0;


    std::uniform_real_distribution<double> distribution(0, 1);
    double r;

    while (i<n) {
        r = distribution(GRIT::generator);
        j += 1+floor(log(1-r)/log(1-edgeProbability));
        while (j >= i && i < n) {
            j -= i;
            i++;
        }
        if (i<n) {
            bool edgeMakesCycle = false;

            for (size_t k=0; k<n; k++) {
                if (k == i || k == j)
                    continue;

                if (hypergraph.getHighestOrderHyperedgeWith(i, k) > 0 && hypergraph.getHighestOrderHyperedgeWith(j, k) > 0) {
                    edgeMakesCycle = true;
                    break;
                }
            }
            if (!edgeMakesCycle)
                hypergraph.addEdge(i, j);
        }
    }
    return hypergraph;
}

GRIT::Hypergraph generateBetaModelHypergraphWithNormalDistributions(size_t n, double edgeMean, double edgeSTD, double triangleMean, double triangleSTD) {
    GRIT::Hypergraph hypergraph(n);

    std::normal_distribution<double> edgePropensityDistribution(edgeMean, edgeSTD);
    std::normal_distribution<double> trianglePropensitiesDistribution(triangleMean, triangleSTD);
    std::uniform_real_distribution<double> uniform01Distribution(0, 1);

    std::vector<double> edgePropensities;
    std::vector<double> trianglePropensities;

    for (size_t i=0; i<n; i++){
        edgePropensities.push_back(edgePropensityDistribution(GRIT::generator));
        trianglePropensities.push_back(trianglePropensitiesDistribution(GRIT::generator));
    }

    double edgeProbability, triangleProbability;

    for (size_t i=0; i<n; i++) {
        for (size_t j=i+1; j<n; j++) {
            edgeProbability = (double) 1/(1+exp(-edgePropensities[i]-edgePropensities[j]));
            if (uniform01Distribution(GRIT::generator) <= edgeProbability)
                hypergraph.addEdge(i, j);

            for (size_t k=j+1; k<n; k++) {
                triangleProbability = (double) 1/(1+exp(-trianglePropensities[i]-trianglePropensities[j]-trianglePropensities[k]));
                if (uniform01Distribution(GRIT::generator) <= triangleProbability)
                    hypergraph.addTriangle({i, j, k});
            }
        }
    }
    return hypergraph;
}

GRIT::Hypergraph generateMillerConfigurationModelHypergraphWithGeometricDistribution(size_t n, double edgeProbability, double triangleProbability) {
    GRIT::Hypergraph hypergraph(n);

    // Not using lists because of the required shuffling
    std::vector<size_t> edgeStubs;
    std::vector<size_t> triangleWedges;

    size_t occurence;
    std::geometric_distribution<size_t> edgeStubDistribution(edgeProbability);
    std::geometric_distribution<size_t> triangleWedgesDistribution(triangleProbability);


    for (size_t i=0; i<n; i++){
        occurence = edgeStubDistribution(GRIT::generator);
        if (occurence > 0)
            edgeStubs.insert(edgeStubs.end(), occurence, i);

        occurence = triangleWedgesDistribution(GRIT::generator);
        if (occurence > 0)
            triangleWedges.insert(triangleWedges.end(), occurence, i);
    }

    // Mersenne twister does not seem compatible with this function so the rng is not specified
    std::shuffle(edgeStubs.begin(), edgeStubs.end());
    std::shuffle(triangleWedges.begin(), triangleWedges.end());


    size_t stub1;
    auto stubIterator = edgeStubs.begin();
    while (stubIterator != edgeStubs.end()) {

        stub1 = *stubIterator;
        stubIterator++;
        if (stubIterator == edgeStubs.end()) break;

        if (stub1 != *stubIterator && hypergraph.getEdgeMultiplicity(stub1, *stubIterator) == 0)  // no loops and multiedges
            hypergraph.addEdge(stub1, *stubIterator);
        stubIterator++;
    }


    size_t wedge1, wedge2;
    auto wedgeIterator = triangleWedges.begin();
    while (wedgeIterator != triangleWedges.end()) {

        wedge1 = *wedgeIterator;
        wedgeIterator++;
        if (wedgeIterator == triangleWedges.end()) break;

        wedge2 = *wedgeIterator;
        wedgeIterator++;
        if (wedgeIterator == triangleWedges.end()) break;

        // Triangles are stored in sets so duplicates are not added
        if (areAllDifferent<size_t>({wedge1, wedge2, *wedgeIterator}))  // no hyperedge self-loop
            hypergraph.addTriangle({wedge1, wedge2, *wedgeIterator});
        wedgeIterator++;
    }
    return hypergraph;
}

GRIT::Hypergraph generateSBMHypergraph(const std::vector<double>& communitySizes,
        const std::vector<double>& intraCommunityEdgeProbabilities, double interCommunityEdgeProbability,
        const std::vector<double>& intraCommunityTriangleProbabilities, double interCommunityTriangleProbability) {
    size_t n = 0;
    for (auto size: communitySizes) {
        if (size < 1) throw std::logic_error("The size of the communities for the SBM must be at least 1");
        if ((long int) size - size != 0) throw std::logic_error("The community sizes must be integers");
        n += size;
    }
    GRIT::Hypergraph hypergraph(n);

    std::uniform_real_distribution<double> uniform01Distribution(0, 1);

    size_t cumulativePosition = 0;
    size_t i;
    size_t j_community, k_community;
    double hyperedgeProbability;

    std::vector<size_t> vertexGroupIndex(n);
    for (size_t i=0; i<n; i++)
        vertexGroupIndex[i] = findSBMGroupFromVertex(communitySizes, i);

    for (size_t i_community=0; i_community<communitySizes.size(); i_community++) {
        for (size_t communityVertex=0; communityVertex<communitySizes[i_community]; communityVertex++) {
            i = communityVertex+cumulativePosition;

            for (size_t j=i+1; j<n; j++) {
                j_community = vertexGroupIndex[j];
                if (i_community == j_community)
                    hyperedgeProbability = intraCommunityEdgeProbabilities[i_community];
                else
                    hyperedgeProbability = interCommunityEdgeProbability;

                if (uniform01Distribution(GRIT::generator) <= hyperedgeProbability)
                    hypergraph.addEdge(i, j);


                for (size_t k=j+1; k<n; k++) {
                    k_community = vertexGroupIndex[k];
                    if (i_community == j_community && j_community == k_community)
                        hyperedgeProbability = intraCommunityTriangleProbabilities[i_community];
                    else
                        hyperedgeProbability = interCommunityTriangleProbability;

                    if (uniform01Distribution(GRIT::generator) <= hyperedgeProbability)
                        hypergraph.addTriangle({i, j, k});
                }
            }
        }
        cumulativePosition += communitySizes[i_community];
    }
    return hypergraph;
}

template<typename T>
static bool areAllDifferent(const std::vector<T>& elements) {
    bool areAllDifferent = true;
    if (elements.size() < 2) throw std::logic_error("areAllDifferent function requires at least 2 arguments");

    for (size_t i=0; i<elements.size()-1 && areAllDifferent; i++)
        for (size_t j=i+1; j<elements.size() && areAllDifferent; j++)
            areAllDifferent = elements[i] != elements[j];

    return areAllDifferent;
}

static inline size_t choose2(size_t n){
    return n*(n-1)/2;
}

static size_t findSBMGroupFromVertex(const std::vector<double>& communitySizes, size_t index) {
    size_t cumulativePosition = 0;
    size_t group = 0;

    for (size_t size: communitySizes){
        cumulativePosition += size;

        if (index < cumulativePosition)
            break;
        group++;
    }

    return group;
}

static void addEdgesWithProbability(GRIT::Hypergraph& hypergraph, double p) {
    size_t n = hypergraph.getSize();

    size_t i=0;
    size_t j=0;


    std::uniform_real_distribution<double> distribution(0, 1);
    double r;

    while (i<n) {
        r = distribution(GRIT::generator);
        j += 1+floor(log(1-r)/log(1-p));
        while (j >= i && i < n) {
            j -= i;
            i++;
        }
        if (i<n)
            hypergraph.addEdge(i, j);
    }
}


void defineRandomHypergraphFunctions(py::module &m) {
    m.def("generate_independent_hyperedges_hypergraph", &generateIndependentHyperedgesHypergraph);
    m.def("generate_independent_layered_edges_hypergraph", &generateIndependentLayeredEdges);
    m.def("generate_sbm_hypergraph", &generateSBMHypergraph);
    m.def("generate_miller_cm_hypergraph_geometric", &generateMillerConfigurationModelHypergraphWithGeometricDistribution);
    m.def("generate_beta_model_hypergraph_normal", &generateBetaModelHypergraphWithNormalDistributions);
    m.def("generate_independent_hyperedges_only_cycles", &generateIndependentHyperedgeOnlyCycles);
    m.def("generate_independent_hyperedges_no_cycles", &generateIndependentHyperedgeWithoutCycles);
}
