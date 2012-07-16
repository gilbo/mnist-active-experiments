
#include "svmModel.h"

#include <cfloat>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;


SvmModel::SvmModel(int num_features, int num_classes) :
    N_features(num_features),
    N_classes(num_classes)
{
    problem.y       = NULL;
    problem.x       = NULL;
    data_layout     = NULL;
    model           = NULL;
    formatted_datum = NULL;
}

SvmModel::~SvmModel()
{
    freeAllocations();
}

// note T *&ptr is a reference to a pointer
template<class T> inline
void freeArray(T *&ptr) {
    if(ptr != NULL)
        delete[] ptr;
    ptr = NULL;
}

void SvmModel::freeAllocations()
{
    freeArray(problem.y);
    freeArray(problem.x);
    freeArray(data_layout);
    freeArray(formatted_datum);
    
    if(model != NULL)
        svm_free_and_destroy_model(&model);
    model = NULL;
}

void SvmModel::bulkTrain(const DataTable *data)
{
    freeAllocations();
    
    // setup the datum space for classification use
    formatted_datum = new svm_node[N_features + 1];
    for(int i=0; i<N_features; i++)
        formatted_datum[i].index = i;
    formatted_datum[N_features].index = -1;
    
    // fill out the svm_problem struct
    data_layout = new svm_node[data->size() * (N_features + 1)];
    {
        problem.l = data->size();
        problem.y = new double[data->size()];
        // allocate array of pointers
        problem.x = new svm_node*[data->size()];
        
        int count = 0;
        for(DataTable::const_iterator it = data->begin();
            it != data->end(); it++) {
            // where to write in this datum
            svm_node *head = data_layout + (count*(N_features + 1));
            
            // fill out the appropriate information for the problem structure
            problem.y[count] = it->label;
            problem.x[count] = head;
            for(int f=0; f<N_features; f++) {
                head[f].index = f;
                head[f].value = it->features[f];
            }
            head[N_features].index = -1; // mark end of list
            
            count++;
        }
    }
    
    // set the parameters for libsvm
    svm_parameter parameters;
    {
        // type = C_SVC or NU_SVC or ONE_CLASS or EPSILON_SVR or NU_SVR
        parameters.svm_type = C_SVC;
        // kernel = LINEAR or POLY or RBF or SIGMOID
        parameters.kernel_type = LINEAR;
        
        // ONLY RELEVANT FOR CERTAIN KERNEL TYPES
        /*
        parameters.degree = ... // FOR POLY
        parameters.gamma = .... // FOR POLY/RBF/SIGMOID
        parameters.coef0 = .... // FOR POLY/SIGMOID
        */
        parameters.degree = 1.0;
        parameters.gamma = 1.0;
        parameters.coef0 = 1.0;
        
        parameters.cache_size = 100; // MB
        parameters.eps = 0.001; // stopping criterion
        parameters.C = 1.0e0; // FOR C_SVC, EPSILON_SVR, NU_SVR
        //parameters.nu = ... // for NU_SVC, ONE_CLASS, NU_SVR
        //parameters.p = ... // for EPSILON_SVR
        parameters.shrinking = 0; // boolean signal: use shrinking heuristics
        parameters.probability = 1; // boolean signal: create probabilities
        
        // nr_weight is size of subsequent parallel arrays
        // nr_weight = 0 means no desire to change penalty for any class
        parameters.nr_weight = 0;
        /*
        parameters.nr_weight = ... // for C_SVC
        parameters.weight_label = ... // for C_SVC
        parameters.weight = ... // for C_SVC
        */
    }
    const char *error = svm_check_parameter(&problem, &parameters);
    if(error != NULL) {
        model = NULL;
        cerr << "There was a problem with the svm_parameter setting provided:"
             << endl << error << endl;
        return;
    }
    
    // ok, train the model
    model = svm_train(&problem, &parameters);
    
    int perror = svm_check_probability_model(model);
    if(perror == 0) {
        model = NULL;
        cerr << "Probabilities are not available for this model" << endl;
        return;
    }
}

int SvmModel::classify(const vector<double> &datum)
{
    if(model == NULL)
        return -1;
    for(int i=0; i<N_features; i++)
        formatted_datum[i].value = datum[i];
    double result = svm_predict(model, formatted_datum);
    return int(result);
}

double SvmModel::top2entropy(const vector<double> &datum)
{
    if(model == NULL)
        return 0.0;
    
    for(int i=0; i<N_features; i++)
        formatted_datum[i].value = datum[i];
    
    int number_classes_seen = svm_get_nr_class(model);
    double *estimates = new double[number_classes_seen];
    svm_predict_probability(model, formatted_datum, estimates);
    
    double ultProb = -1.0;
    double penultProb = -1.0;
    for(int i=0; i<number_classes_seen; i++) {
        if(estimates[i] > penultProb) {
            if(estimates[i] > ultProb) {
                penultProb = ultProb;
                ultProb = estimates[i];
            } else {
                penultProb = estimates[i];
            }
        }
    }
    
    delete[] estimates;
    
    if(number_classes_seen < 2)
        return 0.0;
    
    // otherwise, compute an actual entropy
    double p = ultProb / (ultProb + penultProb);
    return - p*log(p) - (1.0-p)*log(1.0-p);
}