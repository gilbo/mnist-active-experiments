#!/usr/bin/env python

import sys
import string
module_dir = string.replace(sys.path[0], "/bin", "/python")
sys.path = [module_dir] + sys.path # prepend the directory

try: import backend
except ImportError: # then try augmenting the path with env variable
    print "Could not get the backend C++ module loaded correctly..."
    sys.exit(1)

import pylab;
import scipy;
from scipy import stats;
import math;
from math import sqrt;


def stddev(means, data):
    N = len(data);  # number of data instances
    D = len(means); # number of independent "dimensions"
    
    dev = [0.0 for i in range(0,D)];
    for dat in data:
        for i in range(0,D):
            dev[i] += (dat[i] - means[i])**2;
    for i in range(0,D):
        dev[i] = sqrt(dev[i] / float(N-1));
    return dev;

# note that this assumes that all curves generated have
# identical length and x values
def repeatExperiment(repeats, experiment, init,final,step):
    curves = [ experiment(init, final, step) for counter in range(0, repeats) ];
    vals = map(lambda cv: map(lambda pair: pair[1], cv), curves);
    means = map(lambda x:x/float(repeats), map(sum,zip(*vals)));
    #errs = map(lambda x:2.0*x, stats.sem(vals));
    errs = stddev(means, vals);
    xs = map(lambda pair: pair[0], curves[0]);
    return (xs,means,errs);

repetitions = 8;
samp_init = 10;
samp_final = 100;
batch_size = 10;



xs, means, errs = repeatExperiment(repetitions,
                                   backend.passiveCurveLogistic,
                                   samp_init, samp_final, batch_size);
pylab.plot(xs, means, 'r-')
pylab.errorbar(xs, means, yerr=errs, fmt='ro')

xs, means, errs = repeatExperiment(repetitions,
                                   backend.activeCurveLogistic,
                                   samp_init, samp_final, batch_size);
pylab.plot(xs, means, 'r-.')
pylab.errorbar(xs, means, yerr=errs, fmt='rs')



xs, means, errs = repeatExperiment(repetitions,
                                   backend.passiveCurveSVM,
                                   samp_init, samp_final, batch_size);
pylab.plot(xs, means, 'g-')
pylab.errorbar(xs, means, yerr=errs, fmt='go')

xs, means, errs = repeatExperiment(repetitions,
                                   backend.activeCurveSVM,
                                   samp_init, samp_final, batch_size);
pylab.plot(xs, means, 'g-.')
pylab.errorbar(xs, means, yerr=errs, fmt='gs')



xs, means, errs = repeatExperiment(repetitions,
                                   backend.passiveCurveKnn,
                                   samp_init, samp_final, batch_size);
pylab.plot(xs, means, 'b-')
pylab.errorbar(xs, means, yerr=errs, fmt='bo')

xs, means, errs = repeatExperiment(repetitions,
                                   backend.activeCurveKnn,
                                   samp_init, samp_final, batch_size);
pylab.plot(xs, means, 'b-.')
pylab.errorbar(xs, means, yerr=errs, fmt='bs')








pylab.axis([0,xs[len(xs)-1],0,10000]);
pylab.show()