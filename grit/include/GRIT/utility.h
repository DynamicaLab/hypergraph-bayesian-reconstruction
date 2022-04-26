#ifndef GRIT_UTILITY_H
#define GRIT_UTILITY_H

#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <random>
#include <string>
#include <utility>
#include <vector>


namespace GRIT {


class NOPRIOR{
    public:
        double operator()() const { return 0; };
};



extern std::mt19937 generator;


template <typename T>
using SparseMatrix = std::vector<std::map<size_t, T>>;
template <typename T>
using Matrix = std::vector<std::vector<T>>;

typedef Matrix<size_t> Observations;
typedef std::vector<double> Parameters;


struct Edge: std::pair<size_t, size_t> {
    Edge(const size_t& i, const size_t& j) { this->first = i; this->second = j; }
    Edge(const std::pair<size_t, size_t>& other) { this->first = other.first; this->second = other.second; }

    bool operator==(const Edge& other) {
        return (this->first == other.first && this->second == other.second) ||
                    (this->first == other.second && this->second == other.first);
    }
    bool operator!=(const Edge& other) { return !(*this == other); }
    friend std::ostream& operator <<(std::ostream &stream, const Edge& edge) {
        stream << "(" << edge.first << ", " << edge.second << ")";
        return stream;
    }
};


size_t nchoose2(size_t n);
size_t nchoose3(size_t n);
void createOrEmptyDirectory(const std::string& directory);
long getFileSize(std::string filename);

double truncGammaLogProb(double x, double inf, double a, double b);

size_t drawFromShiftedGeometricDistribution(double p, size_t N);
double drawFromBeta(double a, double b);
double drawFromTruncatedGamma(double inf, double sup, double a, double b, size_t maxit=1e5);
double drawFromLowerTruncatedGamma(double inf, double a, double b, size_t maxit=1e6);
double drawFromLinearDistribution(double inf, double sup, double slope);

double drawFromLowerTruncatedGammaITS(double sup, double a, double b);
double drawFromUpperTruncatedGammaITS(double inf, double a, double b);
double drawFromTruncatedGammaITS(double inf, double sup, double a, double b);

double drawTruncatedGammaWithLinearRS(double inf, double sup, double a, double b, size_t maxit=1e6);
double drawTruncatedGammaWithGammaRS(double inf, double sup, double a, double b, size_t maxit=1e6);
double drawTruncatedGammaWithUniformRS(double inf, double sup, double a, double b, size_t maxit=1e6);

void writeParametersToBinary(const Parameters& parameters, const std::string& filename);

template<typename T> // Doesn't write file in a sparse format
void writeSparseMatrixToBinary(const SparseMatrix<T>& sparseMatrix, const std::string& filename) {
    size_t size = sparseMatrix.size();

    std::ofstream fileStream(filename, std::ios::out|std::ios::binary);
    if (!fileStream.is_open()) throw std::runtime_error("Couldn't open \""+filename+"\" to write sparse matrix in binary.");

    const size_t zero = 0;
    for (auto row: sparseMatrix)
        for (size_t j=0; j<size; j++) {
            if (row.find(j) == row.end())
                fileStream.write((char*) &zero, sizeof(T));
            else
                fileStream.write((char*) &row.at(j), sizeof(T));
        }
    fileStream.close();
}

const double MEAN_MIN = 1e-10;
const double MEAN_MAX = 10000;


} //namespace GRIT

#endif
