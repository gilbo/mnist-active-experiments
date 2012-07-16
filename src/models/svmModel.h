
#pragma once

#include "prelude.h"

#include "dataset.h"

#include "svm.h"

#include <vector>
using std::vector;

class SvmModel
{
public:
    SvmModel(int num_features, int num_classes);
    ~SvmModel();
    
    void bulkTrain(const DataTable *data_);
    
    int classify(const vector<double> &datum);
    
    double top2entropy(const vector<double> &datum);
    
private:
    void freeAllocations();
    
private:
    int N_features;
    int N_classes;
    
    svm_problem problem;
    svm_node    *data_layout; // data copied and formatted for libSVM
    svm_model   *model;
    svm_node    *formatted_datum; // space for a single item...
};