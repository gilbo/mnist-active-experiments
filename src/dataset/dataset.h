
#pragma once

#include "getData.h"
#include "features.h"

#include "lottery.h"

#include <vector>
using namespace std;

struct DataRow
{
    Image           image;
    vector<double>  features;
    int             label;
    int             uid;    // unique id
    
    double          density;
    // used for query selection/active learning
    double          entropy_score;
    double          individual_score; // no intra-batch effects
    double          query_score;
};

class DataTable
{
public:
    DataTable() {}
    DataTable(const DataTable &orig);
    
    // by default, load the entire dataset
    bool loadTrain(int size = 0);
    bool loadTest(int size = 0);
    
    void copyRandomSubset(DataTable &subset, int size);
    
    void removeRandomSubset(DataTable &subset, int size);
    // scores must have been computed before invoking...
    void removeTopScoreSubset(DataTable &subset, int size);
    void removeSubsetByUid(DataTable &subset, const vector<int> &uids);
    
    // does not check for duplicates
    void unionWith(DataTable &other);
    
    // get a histogram of the labels present
    vector<int> labelHistogram() const;
    
private:
    bool createDensities(string filename);
    bool loadDensities(string filename);
    
public:
    size_t size() const {
        return rows.size();
    }
    typedef vector<DataRow>::iterator iterator;
    typedef vector<DataRow>::const_iterator const_iterator;
    iterator begin() { return rows.begin(); }
    const_iterator begin() const { return rows.begin(); }
    iterator end() { return rows.end(); }
    const_iterator end() const { return rows.end(); }
    
    void set(const vector<DataRow> &data) { rows = data; }
    
private:
    vector<DataRow> rows;

};

