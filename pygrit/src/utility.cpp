#include <fstream>
#include <sstream>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

#include "GRIT/utility.h"
#include <boost/math/special_functions/gamma.hpp>


namespace GRIT {
using namespace std;


void createOrEmptyDirectory(const string& directory){
    // Might not work on Windows
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir (directory.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            string fileName(ent->d_name);
            string removedFilePath = directory+'/'+fileName;
            remove(removedFilePath.c_str());
        }
        closedir (dir);
    } else {
        string command = "mkdir "+directory;
        system(command.c_str());
    }
}

void writeParametersToBinary(const Parameters& parameters, const std::string& filename) {
    std::ofstream fileStream(filename, std::ios::out|std::ios::binary);
    if (!fileStream.is_open()) throw std::runtime_error("Couldn't open \""+filename+"\" to write parameters in binary.");

    for (size_t i=0; i<parameters.size(); i++)
        fileStream.write((char*) &parameters[i], sizeof(double));
    fileStream.close();
}

size_t nchoose3(size_t n){
    return n*(n-1)*(n-2)/6;
}

size_t nchoose2(size_t n){
    return n*(n-1)/2;
}

long getFileSize(std::string filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

double truncGammaLogProb(double x, double inf, double a, double b) {
    return (a-1)*log(x) - b*x - boost::math::gamma_p(a, inf/b);
}

double drawFromBeta(double a, double b){
    double x = gamma_distribution<double>(a, 1)(generator);
    double y = gamma_distribution<double>(b, 1)(generator);
    return x/(x+y);
}


static double gammaPDF(double x, double a, double b) {
    return boost::math::gamma_p_derivative(a, x / b) / b;
}

double drawTruncatedGammaWithUniformRS(double inf, double sup, double a, double b, size_t maxit) {
    if (sup <= inf)
        throw std::logic_error("Upper bound of truncated distribution must be superior to the lower bound");

    double mode = (a-1)*b;
    double maxInSupport;

    if (mode > inf && mode < sup)
        maxInSupport = mode;
    else if (gammaPDF(sup, a, b) > gammaPDF(inf, a, b))
        maxInSupport = sup;
    else
        maxInSupport = inf;

    double M = gammaPDF(maxInSupport, a, b);

    uniform_real_distribution<double> uniform01Distribution(0, 1);

    double u, proposal;
    bool sampleFound = false;

    for (size_t i=0; i<maxit && !sampleFound; i++){
        proposal = uniform01Distribution(generator)*(sup-inf)+inf;
        u = uniform01Distribution(generator);

        if (u < gammaPDF(proposal, a, b)/M)
            sampleFound = true;
    }
    if (!sampleFound)
        throw runtime_error("Could not sample from the truncated gamma distribution ["+to_string(inf)+", "+to_string(sup)+"] with the"
            " rejection sampling algorithm using an uniform distribution after "+to_string(maxit)+" iterations."
            "Try adjusting the maximum number of iterations.");

    return proposal;
}

double drawTruncatedGammaWithLinearRS(double inf, double sup, double a, double b, size_t maxit) {
    if (sup <= inf)
        throw std::logic_error("Upper bound of truncated distribution must be superior to the lower bound");

    double interval = sup-inf;
    double pdfInf(gammaPDF(inf, a, b)), pdfSup(gammaPDF(sup, a, b));

    double slope = (pdfSup - pdfInf)/interval;
    double maxSlope = slope > 0 ? pdfSup/interval : pdfInf/interval;

    double normalizedSlope = slope/maxSlope;
    auto linearPdf = [&](double x){ return .5*(1+normalizedSlope*( 2*(x-inf)/interval-1) ); };

    double mode = (a-1)*b;
    double maxInSupport;

    if (mode > inf && mode < sup)
        maxInSupport = mode;
    else if (pdfSup > pdfInf)
        maxInSupport = sup;
    else
        maxInSupport = inf;

    double M = gammaPDF(maxInSupport, a, b);
    M = slope > 0 ? M/linearPdf(inf) : M/linearPdf(sup);

    uniform_real_distribution<double> uniform01Distribution(0, 1);

    double u, proposal;

    for (size_t i=0; i<maxit; i++){
        proposal = drawFromLinearDistribution(inf, sup, normalizedSlope);
        u = uniform01Distribution(generator);

        if (u < gammaPDF(proposal, a, b)/M/linearPdf(proposal))
            return proposal;
    }
    return -1;
}

double drawTruncatedGammaWithGammaRS(double inf, double sup, double a, double b, size_t maxit) {
    if (sup <= inf)
        throw std::logic_error("Upper bound of truncated distribution must be superior to the lower bound");


    auto proposalDistribution = gamma_distribution<double>(a, b);
    double proposal;

    for (size_t i=0; i<maxit; i++){
        proposal = proposalDistribution(GRIT::generator);

        if (proposal > inf && proposal < sup)
            return proposal;
    }
    return -1;
}

double drawFromLowerTruncatedGammaITS(double inf, double a, double b) {
    double u = uniform_real_distribution<double>(0, 1)(GRIT::generator);
    if (u == 0)
        return inf;

    double rescaledDraw = u*boost::math::gamma_q(a, inf/b);
    double sampled_parameter = boost::math::gamma_p_inv(a, rescaledDraw);
    return sampled_parameter*b;
}

double drawFromUpperTruncatedGammaITS(double sup, double a, double b) {
    double u = uniform_real_distribution<double>(0, 1)(GRIT::generator);
    if (u == 0)
        return 0;
    if (u == 1)
        return sup;

    double rescaledDraw = u*boost::math::gamma_p(a, sup/b);
    double sampled_parameter = boost::math::gamma_p_inv(a, rescaledDraw);
    return sampled_parameter*b;
}

double drawFromTruncatedGammaITS(double inf, double sup, double a, double b) {
    double cdfInf = boost::math::gamma_p(a, inf/b);
    double cdfSup = boost::math::gamma_p(a, sup/b);


    size_t tries = 100;
    for (size_t i=0; i<tries; i++) {
        double u = uniform_real_distribution<double>(0, 1)(GRIT::generator);

        try {
            double rescaledDraw = u*(cdfSup-cdfInf) + cdfInf;
            double sampled_parameter = b * boost::math::gamma_p_inv(a, rescaledDraw);
            if (! (sampled_parameter > inf && sampled_parameter < sup) )
                continue;
            return sampled_parameter;
        } catch (std::overflow_error&) { }
    }
    return -1;
}

double drawFromTruncatedGamma(double inf, double sup, double a, double b, size_t maxit) {
    double rejectionProbability = inf>0 ?
        boost::math::gamma_p(a, inf/b) + boost::math::gamma_q(a, sup/b) :
        boost::math::gamma_q(a, sup/b);

    double sample=-1;
    if (rejectionProbability < .5/maxit)
        sample = drawTruncatedGammaWithGammaRS(inf, sup, a, b, maxit);

    if (sample == -1)
        sample = drawFromTruncatedGammaITS(inf, sup, a, b);

    if (sample == -1)
        sample = drawTruncatedGammaWithLinearRS(inf, sup, a, b, maxit);

    if (sample == -1)
        throw runtime_error("Could not sample from the truncated gamma distribution ["+to_string(inf)+", "+to_string(sup)+"] with parameters"+
                            " a="+std::to_string(a)+" b="+std::to_string(b)+" with any sampling method." );
    return sample;
}

double drawFromLinearDistribution(double inf, double sup, double slope) {
    if (slope>1 || slope<-1) throw logic_error("The slope must be between -1 and 1");
    double u = uniform_real_distribution<double>(0, 1)(generator);

    double inverseTransformed = (sqrt(slope*slope-2*slope+4*slope*u+1) - 1)/slope;
    return (inverseTransformed/2+0.5) * (sup-inf) + inf;
}

size_t drawFromShiftedGeometricDistribution(double p, size_t N) {
    double u = uniform_real_distribution<double>(0, 1)(GRIT::generator);

    return std::floor( log(1.-u*(1-pow(1-p, N+1)))/log(1-p) + 1 );
}

} //namespace GRIT
