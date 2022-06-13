import numpy
from matplotlib import pyplot
from scipy import stats

import pygrit


def upper_truncgamma(sup, a, b):
    normalisation = stats.gamma.cdf(sup, a, scale=b)

    def f(x):
        return stats.gamma.pdf(x, a, scale=b)/normalisation
    return f


def lower_truncgamma(inf, a, b):
    normalisation = 1-stats.gamma.cdf(inf, a, scale=b)

    def f(x):
        return stats.gamma.pdf(x, a, scale=b)/normalisation
    return f


def truncgamma(inf, sup, a, b):
    normalisation = stats.gamma.cdf(sup, a, scale=b)-stats.gamma.cdf(inf, a, scale=b)

    def f(x):
        return stats.gamma.pdf(x, a, scale=b)/normalisation
    return f


def beta(a, b):
    def f(x):
        return stats.beta.pdf(x, a, b)
    return f


def shifted_geometric(p, N):
    def f(x):
        return (1-p)**(x-2)*p / (1-(1-p)**(N-1))
    return f


def linear(xmin, xmax, slope):
    def f(x):
        rescaled_x = 2*(x-xmin)/(xmax-xmin) - 1
        return (1+slope*rescaled_x)/(xmax-xmin)
    return f


N = 100000
geom_N = 10
maxit = int(1e6)
a, b = 3, 1/2
slope = -0.3
#p = 0.85
p = 0.1
trunc_bound = [0.1, 1]
inf, sup = 5, 10

# grit.sample_from_truncgamma
setups = [("trunc gamma", [
                    # (pygrit.sample_from_truncgamma_its, (*trunc_bound, a, b), "ITS"),
                    # (pygrit.sample_from_truncgamma_gamma_rs, (*trunc_bound, a, b, maxit), "Gamma RS"),
                    # (pygrit.sample_from_truncgamma_uniform_rs, (*trunc_bound, a, b, maxit), "Uniform RS"),
                    (pygrit.sample_from_truncgamma_linear_rs, (*trunc_bound, a, b, maxit), "Linear RS")
                ],
                truncgamma(*trunc_bound, a, b)),
          ("linear", [
              (pygrit.sample_from_linear, (*trunc_bound, slope), "ITS")
              ],
              linear(*trunc_bound, slope)),
          ("trunc geometric", [
                    (pygrit.sample_from_shifted_geometric, (p, geom_N), "Shifted geometric ITS")
                ],
                shifted_geometric(p, geom_N)),
          ("lower", [
                    (pygrit.sample_from_lower_truncgamma_its, (inf, a, b), "ITS")
                ],
                lower_truncgamma(inf, a, b)),
          ("upper", [
                    (pygrit.sample_from_upper_truncgamma_its, (sup, a, b), "ITS")
                ],
                upper_truncgamma(sup, a, b)),
          ("beta", [
                   (pygrit.sample_from_beta, (a, b), "Beta thing")
                   ],
                beta(a, b))
          ]


for name, f_to_test, density in setups:
    labels = []
    samples = []
    for f, args, f_name in f_to_test:
        labels.append(f_name)
        samples.append([f(*args) for i in range(N)])

    if "geometric" in name:
        pyplot.hist(samples, bins=numpy.arange(1.5, geom_N+.5), density=True, label=labels)
    else:
        pyplot.hist(samples, bins=30//len(f_to_test), density=True, label=labels)

    xvalues = numpy.linspace(.9*min(samples[0]), 1.1*max(samples[0]), 1000)
    if "geometric" in name:
        xvalues = numpy.linspace(1.5, geom_N+.5, 200)
    pyplot.plot(xvalues, density(xvalues), color='k')

    pyplot.title(name)
    pyplot.legend()
    pyplot.tight_layout()
    pyplot.show()
