
#pragma once

#include "prelude.h"

#include "dataset.h"

#include <vector>
using std::vector;

class KnnModel
{
public:
    KnnModel(int num_features, int num_classes);
    
    void bulkTrain(const DataTable *data_);
    
    int classify(const vector<double> &datum);
    
    double top2entropy(const vector<double> &datum);
    
private:
    
private:
    int N_features;
    int N_classes;
    
    // raw data
    const DataTable *data;
};