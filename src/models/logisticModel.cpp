
#include "logisticModel.h"
#include <cfloat>

#include "lbfgs.h"

#include <iostream>
using std::cout;
using std::endl;

#include <cmath>
using std::exp;
using std::log;



LogisticModel::LogisticModel(int num_features, int num_classes) :
    N_features(num_features),
    N_classes(num_classes),
    weights(num_classes)
{
    for(int c=0; c<num_classes; c++)
        weights[c].assign(num_features, 0.0);
}

void LogisticModel::bulkTrain(const DataTable *data)
{
    int N_var = N_classes * N_features;
    data_ptr = data;
    
    lbfgsfloatval_t *x = lbfgs_malloc(N_var);
    for(int i=0; i<N_var; i++)
        x[i] = 0.0;
    
    lbfgs_parameter_t param;
    lbfgs_parameter_init(&param);
    param.max_iterations = 30;
    param.epsilon = 1.0e-02;
    
    int ret = lbfgs(
        N_var, // the number of variables
        x, // the array of variables (allocated by lbfgs_malloc)
        NULL, // the objective function value at optimum
        lbfgsEvaluate, // callback to evaluate function and gradient
        NULL, // callback to receive a progress report
        (void *)(this), // data passed to the callbacks...
        &param // L-BFGS parameters
    );
    
    if(ret) {} // prevent compiler warning
           
    //cout << "L-BFGS optimization terminated with status code = "
    //     << ret << endl;
    
    lbfgs_free(x);
}

double LogisticModel::lbfgsEvaluate(
    void            *instance,
    const double    *point,
    double          *grad,
    const int       Nvar,
    const double    step
) {
    //cout << "step size: " << step << endl;
    
    // retreive the object...
    LogisticModel *model = (LogisticModel *)(instance);
    // and pack it with the new weight values...
    int i=0;
    for(int c=0; c<model->N_classes; c++)
        for(int f=0; f<model->N_features; f++)
            model->weights[c][f] = point[i++];
    
    vector< vector<double> > gradient;
    model->computeGradient(gradient, model->data_ptr);
    // and pack it into the provided space
    i = 0;
    for(int c=0; c<model->N_classes; c++)
        for(int f=0; f<model->N_features; f++)
            grad[i++] = gradient[c][f];
    
    double likelihood = model->computeLikelihood(model->data_ptr);
    
    //cout << "neg likelihood: " << likelihood << endl;
    
    return likelihood;
}

double LogisticModel::computeLikelihood(const DataTable *data)
{
    double log_likelihood = 0.0;
    
    vector<double> probs(N_classes);
    for(DataTable::const_iterator it = data->begin();
        it != data->end(); it++) {
        int c = it->label;
        predictClass(probs, it->features);
        log_likelihood += log(probs[c]);
        /*if(d % 1000 == 0) {
            cout << "data instance " << it->uid
                 << ", class " << c
                 << ", likelihood: " << log(probs[c]) << endl;
            cout << "probs" << endl;
            for(int c=0; c<N_classes; c++) {
            cout << "    " << c << ": " << probs[c] << endl;
            }
        }*/
    }
    
    return -log_likelihood / data->size();
}

void LogisticModel::computeGradient(
        vector< vector<double> > &gradient, // parallel size to weights
        const DataTable *data
) {
    vector< vector<double> > raw_counts(N_classes);
    vector< vector<double> > weighted_counts(N_classes);
    gradient.resize(N_classes);
    for(int c=0; c<N_classes; c++) {
        raw_counts[c].assign(N_features, 0.0);
        weighted_counts[c].assign(N_features, 0.0);
        gradient[c].resize(N_features);
    }
    
    vector<double> probs(N_classes);
    
    for(DataTable::const_iterator it = data->begin();
        it != data->end(); it++) {
        // get the true label, and predictions
        int c = it->label;
        predictClass(probs, it->features);
        // add to the raw count and weighted counts
        for(int f=0; f<N_features; f++) {
            raw_counts[c][f]        += it->features[f];
            weighted_counts[c][f]   += probs[c] * it->features[f];
        }
    }
    
    // using these tabulations, we can now produce a gradient.
    // To try to keep things reasonable, we divide the gradient
    // length by the data size
    for(int c=0; c<N_classes; c++)
        for(int f=0; f<N_features; f++)
            gradient[c][f] =
                - (raw_counts[c][f] - weighted_counts[c][f]) / data->size();
}

// compute a distribution over classes given a datum
void LogisticModel::predictClass(vector<double> &probs,
                                 const vector<double> &datum)
{
    vector<double> scores(N_classes, 0.0);
    
    double sumScore = 0.0;
    for(int c=0; c<N_classes; c++) {
        for(int f=0; f<N_features; f++)
            scores[c] += weights[c][f] * datum[f];
        scores[c] = exp(scores[c]);
        sumScore += scores[c];
    }
    
    // now normalize
    for(int c=0; c<N_classes; c++) {
        probs[c] = scores[c] / sumScore;
    }
}

int LogisticModel::classify(const vector<double> &datum)
{
    vector<double> probs(N_classes);
    predictClass(probs, datum);
    
    double  maxProb  = 0.0;
    int     maxClass = 0;
    for(int c=0; c<N_classes; c++) {
        if(probs[c] > maxProb) {
            maxProb = probs[c];
            maxClass = c;
        }
    }
    
    return maxClass;
}

double LogisticModel::top2entropy(const vector<double> &datum)
{
    vector<double> probs(N_classes);
    predictClass(probs, datum);
    
    double  topProb[2] = { 0, 0 };
    for(int c=0; c<N_classes; c++) {
        if(probs[c] > topProb[0]) {
            if(probs[c] > topProb[1]) {
                topProb[0] = topProb[1];
                topProb[1] = probs[c];
            } else {
                topProb[0] = probs[c];
            }
        }
    }
    
    // renormalize top probabilities?
    double entropy = - topProb[0] * log(topProb[0])
                     - topProb[1] * log(topProb[1]);
    
    return entropy;
}

