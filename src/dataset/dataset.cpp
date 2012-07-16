
#include "dataset.h"

#include "uid.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <cfloat>
#include <set>
using namespace std;

namespace {
    //string train_densities_filename = "data/train-invL2sq-densities-txt";
    //string test_densities_filename = "data/test-invL2sq-densities-txt";
    
    string train_densities_filename = "data/train-cos-densities-txt";
    string test_densities_filename = "data/test-cos-densities-txt";


    bool fileExists(string filename)
    {
        ifstream in(filename.c_str());
        return bool(in);
    }
}

DataTable::DataTable(const DataTable &orig) : rows(orig.rows) {}

bool DataTable::createDensities(string filename)
{
    cout << "Now computing density file " << filename << endl;
    // first compute the densities, then store to disk
    for(uint i=0; i<rows.size(); i++) {
        double density = 0.0;
        for(uint j=0; j<rows.size(); j++) {
            if(i==j)    continue;
            //double sim = feat_sim_invL2sq(rows[i].features, rows[j].features);
            double sim = feat_sim_cos(rows[i].features, rows[j].features);
            density += sim;
        }
        if(i%1000 == 0) {
            cout << "densities computed for " << i << " points" << endl;
        }
        rows[i].density = density;
    }
    
    ofstream out(filename.c_str());
    if(!out) {
        cerr << "Error opening density file " << filename << " to write"
             << endl;
        return false;
    }
    
    for(uint i=0; i<rows.size(); i++) {
        out << rows[i].density << endl;
    }
    if(!out) {
        cerr << "Error while writing densities file: " << filename << endl;
        return false;
    }
    
    return true;
}

bool DataTable::loadDensities(string filename)
{
    ifstream in(filename.c_str());
    if(!in) {
        cerr << "Unable to read densities file: " << filename << endl;
        return false;
    }
    
    for(uint i=0; i<rows.size(); i++) {
        in >> rows[i].density;
    }
    if(!in) {
        cerr << "Error while reading densities file: " << filename << endl;
        return false;
    }
    
    return true;
}


bool DataTable::loadTrain(int size)
{
    vector<Image>   images;
    vector<int>     labels;
    if(!trainingData(images, labels))
        return false;
    
    if(size > 0)
        images.resize(min(int(images.size()), size));
    size = images.size();
    
    rows.resize(size);
    for(int i=0; i<size; i++) {
        rows[i].uid   = UID::create();
        rows[i].image = images[i];
        rows[i].label = labels[i];
        extractFeatures(rows[i].features, images[i]);
    }
    
    // load in the densities
    if(!fileExists(train_densities_filename)) {
        if(!createDensities(train_densities_filename))
            return false;
    }
    if(!loadDensities(train_densities_filename)) {
        return false;
    }
    
    return true;
}

bool DataTable::loadTest(int size)
{
    vector<Image>   images;
    vector<int>     labels;
    if(!testingData(images, labels))
        return false;
    
    if(size > 0)
        images.resize(min(int(images.size()), size));
    size = images.size();
    
    rows.resize(size);
    for(int i=0; i<size; i++) {
        rows[i].uid   = UID::create();
        rows[i].image = images[i];
        rows[i].label = labels[i];
        extractFeatures(rows[i].features, images[i]);
    }
    
    // load in the densities
    if(!fileExists(test_densities_filename)) {
        if(!createDensities(test_densities_filename))
            return false;
    }
    if(!loadDensities(test_densities_filename)) {
        return false;
    }
    
    return true;
}

void DataTable::removeRandomSubset(DataTable &subset, int size)
{
    if(size > int(rows.size())) return;
    
    // first run a lottery
    Lottery lottery(rows.size());
    lottery.drawN(size);
    
    // now extract the rows comprising the subset
    // and compact the remaining rows in one unified pass
    subset.rows.resize(size);
    uint here_w = 0;
    uint other_w = 0;
    for(uint read = 0; read<rows.size(); read++) {
        if(lottery.isDrawn(read)) {
            subset.rows[other_w] = rows[read];
            other_w++;
        } else {
            rows[here_w] = rows[read];
            here_w++;
        }
    }
    rows.resize(here_w);
}

void DataTable::removeTopScoreSubset(DataTable &subset, int size)
{
    if(size > int(rows.size())) return;
    
    // NOTE: this should probably use a heap, but I don't think it
    // makes enough of a difference to care right now.
    
    // compute the top k scoring rows
    vector<int> top_i(size, -1);
    vector<double> top_score(size, -DBL_MAX);
    for(uint i=0; i<rows.size(); i++) {
        double score = rows[i].query_score;
        if(score > top_score[0]) {
            int idx = 1;
            for(; idx < size && score > top_score[idx]; idx++) {
                top_i[idx-1]        = top_i[idx];
                top_score[idx-1]    = top_score[idx-1];
            }
            top_i[idx-1]        = i;
            top_score[idx-1]    = score;
        }
    }
    
    // sort the list of row numbers
    sort(top_i.begin(), top_i.end());
    
    // now split the dataset
    subset.rows.resize(size);
    uint here_w     = 0;
    uint other_w    = 0;
    uint idx_r      = 0;
    for(uint read = 0; read<rows.size(); read++) {
        if(int(read) == top_i[idx_r]) { // then extract
            subset.rows[other_w] = rows[read];
            other_w++;
            idx_r++;
        } else {
            rows[here_w] = rows[read];
            here_w++;
        }
    }
    rows.resize(here_w);
}

void DataTable::removeSubsetByUid(DataTable &subset, const vector<int> &uids_)
{
    std::set<int> uids(uids_.begin(), uids_.end());
    
    // split the dataset
    int N = uids_.size();
    subset.rows.resize(N);
    int here_w      = 0;
    int other_w     = 0;
    for(uint read = 0; read<rows.size(); read++) {
        if(uids.count(rows[read].uid) > 0) { // then extract
            subset.rows[other_w] = rows[read];
            other_w++;
        } else {
            rows[here_w] = rows[read];
            here_w++;
        }
    }
    rows.resize(here_w);
}

void DataTable::unionWith(DataTable &other)
{
    int addsize = other.rows.size();
    int oldsize = rows.size();
    int newsize = oldsize + addsize;
    rows.resize(newsize);
    for(int i=0; i<addsize; i++)
        rows[oldsize+i] = other.rows[i];
}

vector<int> DataTable::labelHistogram() const
{
    vector<int> ret(10, 0); // 10 entries, all 0 to start with
    for(uint i=0; i<rows.size(); i++)
        ret[rows[i].label]++;
    return ret;
}






