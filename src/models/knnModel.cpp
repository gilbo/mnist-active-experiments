
#include "knnModel.h"

#include <cfloat>

#include <Eigen/Core>
#include <Eigen/SVD>

#include <iostream>
using std::cout;
using std::endl;


KnnModel::KnnModel(int num_features, int num_classes) :
    N_features(num_features),
    N_classes(num_classes)
{
    
}

void KnnModel::bulkTrain(const DataTable *data_)
{
    data = data_;
    
    // compute the data mean
    /*vector<double> mean(N_features, 0.0);
    for(uint d=0; d<data.size(); d++)
        for(int f=0; f<N_features; f++)
            mean[f] += data[d][f];
    for(int f=0; f<N_features; f++)
        mean[f] /= data.size();
    
    // fill this matrix with the mean-centered data
    Eigen::MatrixXf centered(data.size(), N_features);
    for(uint d=0; d<data.size(); d++)
        for(int f=0; f<N_features; f++)
            centered(d,f) = data[d][f] - mean[f];
    
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(centered, ComputeThinV);
    cout << "singular values are... " << svd.singularValues() << endl;
    */
    //Eigen::MatrixXf principalVecs = svd.matrixV();
    
    // build any acceleration structure...
}

int KnnModel::classify(const vector<double> &datum)
{
    if(data->size() < 3)    return -1;
    
    double closest_dist2[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
    int closest_labels[3]   = {-1,-1,-1};
    
    for(DataTable::const_iterator it = data->begin();
        it != data->end(); it++) {
        double dist2 = 0.0;
        for(int f=0; f<N_features; f++) {
            double diff = datum[f] - it->features[f];
            dist2 += diff * diff;
        }
        
        if(dist2 < closest_dist2[0]) {
            closest_dist2[0]    = dist2;
            closest_labels[0]   = it->label;
            for(uint i=1; i<3; i++) {
                if(dist2 < closest_dist2[i]) {
                    closest_dist2[i-1]  = closest_dist2[i];
                    closest_dist2[i]    = dist2;
                    closest_labels[i-1] = closest_labels[i];
                    closest_labels[i]   = it->label;
                }
                else
                    break;
            }
        }
    }
        
    if(closest_labels[0] == closest_labels[1])
        return closest_labels[1];
    else
        return closest_labels[2];
}

double KnnModel::top2entropy(const vector<double> &datum)
{
    if(data->size() < 3)    return -1;
    
    double closest_dist2[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
    int closest_labels[3]   = {-1,-1,-1};
    
    for(DataTable::const_iterator it = data->begin();
        it != data->end(); it++) {
        double dist2 = 0.0;
        for(int f=0; f<N_features; f++) {
            double diff = datum[f] - it->features[f];
            dist2 += diff * diff;
        }
        
        if(dist2 < closest_dist2[0]) {
            closest_dist2[0]    = dist2;
            closest_labels[0]   = it->label;
            for(uint i=1; i<3; i++) {
                if(dist2 < closest_dist2[i]) {
                    closest_dist2[i-1]  = closest_dist2[i];
                    closest_dist2[i]    = dist2;
                    closest_labels[i-1] = closest_labels[i];
                    closest_labels[i]   = it->label;
                }
                else
                    break;
            }
        }
    }
    
    // certain label
    if(closest_labels[0] == closest_labels[1] &&
       closest_labels[0] == closest_labels[2])
        return 0.0;
    else if(closest_labels[0] == closest_labels[1] ||
            closest_labels[0] == closest_labels[2] ||
            closest_labels[1] == closest_labels[3])
        return -(0.66*log(0.66) + 0.33*log(0.33));
    else
        return -log(0.33);
}

