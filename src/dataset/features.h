
#pragma once

#include "image.h"

#include<vector>
using std::vector;

#include<cmath>
using std::sqrt;



const int N_FEATURES = Image::WIDTH * Image::HEIGHT + 1;

// assumes the above amount of space has been allocated for features
void extractFeatures(vector<double> &features, const Image &image);

static inline
double feat_dist_L2sq(const vector<double> &a, const vector<double> &b)
{
    double sumsq = 0.0;
    int N = a.size();
    for(int i=0; i<N; i++) {
        double d = a[i] - b[i];
        sumsq += d*d;
    }
    return sumsq;
}

static inline
double feat_dist_L2(const vector<double> &a, const vector<double> &b)
{
    return sqrt(feat_dist_L2sq(a, b));
}

static inline
double feat_sim_invL2sq(const vector<double> &a, const vector<double> &b)
{
    double sumsq = 0.0;
    int N = a.size();
    for(int i=0; i<N; i++) {
        double d = a[i] - b[i];
        sumsq += d*d;
    }
    return 1.0 / sumsq;
}

static inline
double feat_sim_cos(const vector<double> &a, const vector<double> &b)
{
    double normA = 0.0;
    double normB = 0.0;
    double dot   = 0.0;
    int N = a.size();
    for(int i=0; i<N; i++) {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    return dot / sqrt(normA * normB);
}