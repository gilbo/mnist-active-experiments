
#pragma once

#include "dataset.h"

#include <cfloat>

class EditDatum
{
public:
    EditDatum() {}
    EditDatum(const DataRow &row) {
        image0         = row.image;
        features0      = row.features;
        label0_true    = row.label;
        density0       = row.density;
        uid0           = row.uid;
    }
    
    template<class Model>
    void makeGuess(Model &model) {
        label0_guess = model.classify(features0);
    }
    
    void simulateEdit(const DataTable &gold) {
        // use the dataset as a gold standard as follows
        // if the guess is wrong, then choose the
        // closest instance with the guessed label
        
        label0_solicited = label0_true;
        
        has_mod = (label0_solicited != label0_guess);
        if(!has_mod)    return;
        
        double min_dist = DBL_MAX;
        const DataRow *min_inst = NULL;
        for(DataTable::const_iterator it=gold.begin(); it!=gold.end(); it++) {
            if(it->label != label0_guess)   continue;
            
            double dist = feat_dist_L2sq(features0, it->features);
            if(dist < min_dist) {
                min_dist = dist;
                min_inst = &(*it);
            }
        }
        
        if(min_inst == NULL) {
            has_mod = false;
            return;
        }
        
        normed_mod_image = raw_mod_image = min_inst->image;
        mod_features = min_inst->features;
        mod_label = min_inst->label;
    }
    
public:
    // stimulus datum
    Image image0;
    vector<double> features0;
    int uid0; // has a unique id
    double density0;
    int label0_guess;
    int label0_true;
    int label0_solicited;
    
    bool has_mod;
    
    // modified datum
    Image raw_mod_image;
    Image normed_mod_image;
    vector<double> mod_features;
    int mod_label;
};





