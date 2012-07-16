
#pragma once

#include "prelude.h"

#include "dataset.h"

#include <vector>
using std::vector;

class LogisticModel
{
public:
    LogisticModel(int num_features, int num_classes);
    
    void bulkTrain(const DataTable *data);
    
    int classify(const vector<double> &datum);
    
    double top2entropy(const vector<double> &datum);
    
private:
    // compute a distribution over classes given a datum
    void predictClass(vector<double> &probs,
                      const vector<double> &datum);
    
    double computeLikelihood(const DataTable *data);
    
    void computeGradient(
        vector< vector<double> > &gradient, // parallel size to weights
        const DataTable *data
    );
    
    static double lbfgsEvaluate(
        void            *instance,
        const double    *point,
        double          *gradient,
        const int       Nvar,
        const double    step
    );
    const DataTable *data_ptr;
    
private:
    int N_features;
    int N_classes;
public:
    vector< vector<double> >    weights;
};