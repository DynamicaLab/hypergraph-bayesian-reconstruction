from libc.math cimport exp, log, lgamma
cimport numpy as np


ctypedef np.double_t FLOAT_TYPE
ctypedef np.int32_t INT_TYPE


cdef log_sum_exp(FLOAT_TYPE y[3]):
    cdef FLOAT_TYPE ymax = y[0]
    if y[1] > ymax:
        ymax = y[1]
    if y[2] > ymax:
        ymax = y[2]

    cdef FLOAT_TYPE total=0
    for i in range(3):
        total += exp(y[i]-ymax)
    return ymax+log(total)


cpdef get_neg_loglikelihood(np.ndarray[FLOAT_TYPE, ndim=1] parameters, np.ndarray[FLOAT_TYPE, ndim=1] occurences, np.ndarray[FLOAT_TYPE, ndim=1] values):
    cdef FLOAT_TYPE loglikelihood = 0
    cdef FLOAT_TYPE elements_to_sum[3]

    for i in range(len(occurences)):
        for j in range(3):
            elements_to_sum[j] = log(parameters[j]) + values[i]*log(parameters[3+j]) - parameters[3+j] - lgamma(values[i]+1)

        loglikelihood += occurences[i]*log_sum_exp(elements_to_sum)
    return -loglikelihood


cpdef rho_constraint(np.ndarray[FLOAT_TYPE, ndim=1] parameters):
    cdef FLOAT_TYPE total = 0
    for i in range(3):
        total += parameters[i]
    return total-1
