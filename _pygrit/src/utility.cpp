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

double truncGammaLogProb(double x, double inf, double k, double theta) {
    return (k-1)*log(x) - theta*x - boost::math::gamma_p(k, inf/theta);
}

double drawFromBeta(double k, double theta){
    double x = gamma_distribution<double>(k, 1)(generator);
    double y = gamma_distribution<double>(theta, 1)(generator);
    return x/(x+y);
}


static double gammaPDF(double x, double k, double theta) {
    return boost::math::gamma_p_derivative(k, x / theta) / theta;
}

double drawTruncatedGammaWithUniformRS(double inf, double sup, double k, double theta, size_t maxit) {
    if (sup <= inf)
        throw std::logic_error("Upper bound of truncated distribution must be superior to the lower bound");

    double mode = (k-1)*theta;
    double maxInSupport;

    if (mode > inf && mode < sup)
        maxInSupport = mode;
    else if (gammaPDF(sup, k, theta) > gammaPDF(inf, k, theta))
        maxInSupport = sup;
    else
        maxInSupport = inf;

    double M = gammaPDF(maxInSupport, k, theta);

    uniform_real_distribution<double> uniform01Distribution(0, 1);

    double u, proposal;
    bool sampleFound = false;

    for (size_t i=0; i<maxit && !sampleFound; i++){
        proposal = uniform01Distribution(generator)*(sup-inf)+inf;
        u = uniform01Distribution(generator);

        if (u < gammaPDF(proposal, k, theta)/M)
            sampleFound = true;
    }
    if (!sampleFound)
        throw runtime_error("Could not sample from the truncated gamma distribution ["+to_string(inf)+", "+to_string(sup)+"] with the"
            " rejection sampling algorithm using an uniform distribution after "+to_string(maxit)+" iterations."
            "Try adjusting the maximum number of iterations.");

    return proposal;
}

double drawTruncatedGammaWithLinearRS(double inf, double sup, double k, double theta, size_t maxit) {
    if (sup <= inf)
        throw std::logic_error("Upper bound of truncated distribution must be superior to the lower bound");

    double interval = sup-inf;
    double pdfInf(gammaPDF(inf, k, theta)), pdfSup(gammaPDF(sup, k, theta));

    double slope = (pdfSup - pdfInf)/interval;
    double maxSlope = slope > 0 ? pdfSup/interval : pdfInf/interval;

    double normalizedSlope = slope/maxSlope;
    auto linearPdf = [&](double x){ return .5*(1+normalizedSlope*( 2*(x-inf)/interval-1) ); };

    double mode = (k-1)*theta;
    double maxInSupport;

    if (mode > inf && mode < sup)
        maxInSupport = mode;
    else if (pdfSup > pdfInf)
        maxInSupport = sup;
    else
        maxInSupport = inf;

    double M = gammaPDF(maxInSupport, k, theta);
    M = slope > 0 ? M/linearPdf(inf) : M/linearPdf(sup);

    uniform_real_distribution<double> uniform01Distribution(0, 1);

    double u, proposal;

    for (size_t i=0; i<maxit; i++){
        proposal = drawFromLinearDistribution(inf, sup, normalizedSlope);
        u = uniform01Distribution(generator);

        if (u < gammaPDF(proposal, k, theta)/M/linearPdf(proposal))
            return proposal;
    }
    return -1;
}

double drawTruncatedGammaWithGammaRS(double inf, double sup, double k, double theta, size_t maxit) {
    if (sup <= inf)
        throw std::logic_error("Upper bound of truncated distribution must be superior to the lower bound");


    auto proposalDistribution = gamma_distribution<double>(k, theta);
    double proposal;

    for (size_t i=0; i<maxit; i++){
        proposal = proposalDistribution(GRIT::generator);

        if (proposal > inf && proposal < sup)
            return proposal;
    }
    return -1;
}

double drawFromLowerTruncatedGammaITS(double inf, double k, double theta) {
    double u = uniform_real_distribution<double>(0, 1)(GRIT::generator);
    if (u == 0)
        return inf;

    double rescaledDraw = u*boost::math::gamma_q(k, inf/theta);
    double sampled_parameter = boost::math::gamma_p_inv(k, rescaledDraw);
    return sampled_parameter*theta;
}

double drawFromUpperTruncatedGammaITS(double sup, double k, double theta) {
    double u = uniform_real_distribution<double>(0, 1)(GRIT::generator);
    if (u == 0)
        return 0;
    if (u == 1)
        return sup;

    double rescaledDraw = u*boost::math::gamma_p(k, sup/theta);
    double sampled_parameter = boost::math::gamma_p_inv(k, rescaledDraw);
    return sampled_parameter*theta;
}

double drawFromTruncatedGammaITS(double inf, double sup, double k, double theta) {
    double cdfInf = boost::math::gamma_p(k, inf/theta);
    double cdfSup = boost::math::gamma_p(k, sup/theta);


    size_t tries = 100;
    for (size_t i=0; i<tries; i++) {
        double u = uniform_real_distribution<double>(0, 1)(GRIT::generator);

        try {
            double rescaledDraw = u*(cdfSup-cdfInf) + cdfInf;
            double sampled_parameter = theta * boost::math::gamma_p_inv(k, rescaledDraw);
            if (! (sampled_parameter > inf && sampled_parameter < sup) )
                continue;
            return sampled_parameter;
        } catch (std::overflow_error&) { }
    }
    return -1;
}

double drawFromTruncatedGamma(double inf, double sup, double k, double theta, size_t maxit) {
    double rejectionProbability = inf>0 ?
        boost::math::gamma_p(k, inf/theta) + boost::math::gamma_q(k, sup/theta) :
        boost::math::gamma_q(k, sup/theta);

    double sample=-1;
    if (rejectionProbability < .5/maxit)
        sample = drawTruncatedGammaWithGammaRS(inf, sup, k, theta, maxit);

    if (sample == -1)
        sample = drawFromTruncatedGammaITS(inf, sup, k, theta);

    if (sample == -1)
        sample = drawTruncatedGammaWithLinearRS(inf, sup, k, theta, maxit);

    if (sample == -1)
        throw runtime_error("Could not sample from the truncated gamma distribution ["+to_string(inf)+", "+to_string(sup)+"] with parameters"+
                            " k="+std::to_string(k)+" theta="+std::to_string(theta)+" with any sampling method." );
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

    return std::floor( log(1.-u*(1-pow(1-p, N-1)))/log(1-p) + 2 );
}

} //namespace GRIT
