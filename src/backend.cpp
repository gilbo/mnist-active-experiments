
#include "backend.h"

#include "dataset.h"
#include "logisticModel.h"
#include "knnModel.h"
#include "svmModel.h"

#include "uid.h"

#include "editDatum.h"

#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cfloat>
using namespace std;



int getImageSize() // in count of bytes for buffer
{
    return Image::WIDTH * Image::HEIGHT * 4 * 16;
    // 4 for RGBA
    // 4*4 = 16 for upsampling
}

// used to get an image to display on the python front-end
void getTrainingImage(char *buffer, size_t size, int num)
{
    vector<Image>   images;
    vector<int>     labels;
    if(!trainingData(images, labels))
        return;
    
    if(num < 0 || uint(num) >= images.size())
        return;
    
    const Image &image = images[num];
    
    if(int(size) < getImageSize())
        return;
    
    for(int r=0; r<Image::HEIGHT; r++) {
        for(int c=0; c<Image::WIDTH; c++) {
            int step = Image::WIDTH * 4;
            int roff = r * 4 * step;
            int coff = c * 4;
            
            uchar raw = 255 - image.p[r][c];
            for(int i=0; i<4; i++) {
                for(int j=0; j<4; j++) {
                    int pix = 4 * (roff + coff + i*step + j);
                    buffer[pix + 0] = raw;
                    buffer[pix + 1] = raw;
                    buffer[pix + 2] = raw;
                    buffer[pix + 3] = 255;
                }
            }
        }
    }
}



void printLabelHistogram(const DataTable &data)
{
    vector<int> histo = data.labelHistogram();
    double size = data.size();
    
    double entropy = 0.0;
    for(uint i=0; i<10; i++) {
        double prob = histo[i]/size;
        entropy += (prob > 0)? -(prob * log(prob)) : 0.0;
    }
    
    cout << entropy << " **";
    
    for(uint i=0; i<10; i++) {
        cout << ' ' << histo[i];
    }
    
    cout << " ** " << data.size();
    cout << endl;
}

void printWeights(const LogisticModel &model)
{
    for(int c=0; c<10; c++) {
        for(int f=0; f<28*28; f++) {
            double w = model.weights[c][f];
            if(w < 0.0)
                cout << "-";
            else if(w < 0.2)
                cout << ".";
            else if(w < 0.4)
                cout << "x";
            else
                cout << "X";
            if(f%28 == 27)
                cout << endl;
        }
        cout << endl;
    }
}

int runDevelop()
{
    DataTable train;
    train.loadTrain(1000);
    
    KnnModel model(N_FEATURES, 10/*N_CLASSES*/);
    model.bulkTrain(&train);
    
    // We've trained the model, now let's get some test data and
    // go to town with it.
    //printWeights(model);
    
    DataTable test;
    test.loadTest();
    
    int nTotal = test.size();
    int nRight = 0;
    int nWrong = 0;
    
    int counter = 0;
    for(DataTable::iterator it = test.begin(); it != test.end(); it++) {
        if(counter%1000 == 0) cout << "processed " << counter << endl;
        counter++;
        
        int prediction = model.classify(it->features);
        if(prediction == it->label)
            nRight++;
        else
            nWrong++;
    }
    
    cout << "total number of test images: " << nTotal << endl;
    cout << "        correct predictions: " << nRight << endl;
    cout << "                             ("
         << 100 * double(nRight)/nTotal << "%)" << endl;
    cout << "      incorrect predictions: " << nWrong << endl;
    cout << "                             ("
         << 100 * double(nWrong)/nTotal << "%)" << endl;
    
    
    return 0;
}






template<class Model> vector< pair<double,double> >
passiveCurve(int init, int final, int step)
{
    vector< pair<double, double> > results;
    srand(time(0));
    
    DataTable trainReserve;
    trainReserve.loadTrain();
    
    Model model(N_FEATURES, 10/*N_CLASSES*/);
    
    DataTable test;
    test.loadTest();
    
    
    // setup the inital dataset, and train a model on it
    DataTable train;
    trainReserve.removeRandomSubset(train, init);
    model.bulkTrain(&train);
    printLabelHistogram(train);
    
    
    // now we go into an evaluate->train loop
    while(int(train.size()) <= final) {
    // evaluate
        int correct = 0;
        for(DataTable::iterator it = test.begin(); it != test.end(); it++) {
            if(model.classify(it->features) == it->label)
                correct++;
        }
        results.push_back(pair<double,double>(train.size(),correct));
        cout << correct << endl;
    // abort?
        int oldsize = train.size();
        if(oldsize + step > final)  break;
    // sampling + re-training
        DataTable batch;
        trainReserve.removeRandomSubset(batch, step);
        train.unionWith(batch);
        
        model.bulkTrain(&train);
        printLabelHistogram(train);
    }
    cout << "DONE!" << endl;
    
    return results;
}

vector< pair<double,double> >
passiveCurveLogistic(int init, int final, int step)
{
    return passiveCurve<LogisticModel>(init, final, step);
}

vector< pair<double,double> >
passiveCurveKnn(int init, int final, int step)
{
    return passiveCurve<KnnModel>(init, final, step);
}

vector< pair<double,double> >
passiveCurveSVM(int init, int final, int step)
{
    return passiveCurve<SvmModel>(init, final, step);
}





template<class Model>
void greedyBatchQuerySelection(
    DataTable &batch, DataTable &pool,
    Model &model, int num
) {
    vector<DataRow*> selection;
    
    // fill out entropy values once
    for(DataTable::iterator it = pool.begin(); it != pool.end(); it++) {
        double entropy = model.top2entropy(it->features);
        double density = it->density;
        density = 1.0;
        
        it->entropy_score = entropy;
        it->individual_score = entropy * density;
    }
    
    for(int count = 0; count < num; count++) {
        // choose a maximum scoring item
        double maxScore     = -DBL_MAX;
        DataRow *maxRow     = NULL;
        for(DataTable::iterator it = pool.begin(); it != pool.end(); it++) {
            // find the selected datum which is most similar to this one
            bool skip = false;
            double max_sim = -DBL_MAX;
            for(uint i=0; i<selection.size(); i++)
            {
                if(selection[i]->uid == it->uid)   skip = true;
                double sim = feat_sim_cos(it->features,
                                          selection[i]->features);
                max_sim = max(max_sim, sim);
            }
            if(max_sim == -DBL_MAX)
                max_sim = 0.0;
            
            if(skip)    continue;
            
            double diversity = 1.0 - max_sim;
            double base = it->individual_score;
            const double LAMBDA = 0.5;
            double score = LAMBDA * base + (1.0-LAMBDA) * diversity;
            //score = base;
            
            it->query_score = score;
            if(score > maxScore) {
                maxScore    = score;
                maxRow      = &(*it);
            }
        }
        // add the maximum scoring item to the selection
        selection.push_back(maxRow);
    }
    
    vector<int> uids(num);
    for(int i=0; i<num; i++)
        uids[i] = selection[i]->uid;
    
    pool.removeSubsetByUid(batch, uids);
}

template<class Model>
void dumbBatchQuerySelection(
    DataTable &batch, DataTable &pool,
    Model &model, int num
) {
    // score the remaining training examples for the active learner
    for(DataTable::iterator it = pool.begin(); it != pool.end(); it++) {
        double entropy = model.top2entropy(it->features);
        double density = 1.0;
        double diversity = 1.0;
        double score = entropy * density * diversity;
        
        it->query_score = score;
    }
    
    // then extract the highest scoring rows from the table
    pool.removeTopScoreSubset(batch, num);
}

template<class Model>
void dumbLimitedBatchQuerySelection(
    DataTable &batch, DataTable &pool,
    Model &model, int num
) {
    const int K = 4; // number of items to consider
    DataTable subPool;
    pool.copyRandomSubset(subPool, num*K);
    vector<DataRow*> selection(num); // temp storage
    
    // choose the best option in each group of K items
    DataTable::iterator it = subPool.begin();
    for(int i=0; i<num; i++) {
        double maxScore     = -DBL_MAX;
        DataRow *maxRow     = NULL;
        for(int j=0; j<K; j++) {
            // compute the score...
            double entropy = model.top2entropy(it->features);
            double density = 1.0;
            double score = entropy * density;
            
            if(score > maxScore) {
                maxScore = score;
                maxRow = &(*it);
            }
            it++;
        }
        selection[i] = maxRow;
    }
    
    // extract the selected rows
    vector<int> uids(num);
    for(int i=0; i<num; i++)
        uids[i] = selection[i]->uid;
    pool.removeSubsetByUid(batch, uids);
}

template<class Model> std::vector< std::pair<double,double> >
activeCurve(int init, int final, int step)
{
    vector< pair<double, double> > results;
    srand(time(0));
    
    DataTable trainReserve;
    trainReserve.loadTrain();
    
    Model model(N_FEATURES, 10/*N_CLASSES*/);
    
    DataTable test;
    test.loadTest();
    
    
    // setup the inital dataset, and train a model on it
    DataTable train;
    trainReserve.removeRandomSubset(train, init);
    model.bulkTrain(&train);
    printLabelHistogram(train);
    
    
    // now we go into an evaluate->train loop
    while(int(train.size()) <= final) {
    // evaluate
        int correct = 0;
        for(DataTable::iterator it = test.begin(); it != test.end(); it++) {
            if(model.classify(it->features) == it->label)
                correct++;
        }
        results.push_back(pair<double,double>(train.size(),correct));
        cout << correct << endl;
    // abort?
        int oldsize = train.size();
        if(oldsize + step > final)  break;
    // sampling + re-training
        DataTable batch;
        //greedyBatchQuerySelection(batch, trainReserve, model, step);
        //dumbBatchQuerySelection(batch, trainReserve, model, step);
        dumbLimitedBatchQuerySelection(batch, trainReserve, model, step);
        
        // now, add in the selected batch and re-train
        train.unionWith(batch);
        model.bulkTrain(&train);
        printLabelHistogram(train);
    }
    cout << "DONE!" << endl;
    
    return results;
}

std::vector< std::pair<double,double> >
activeCurveLogistic(int init, int final, int step)
{
    return activeCurve<LogisticModel>(init, final, step);
}

std::vector< std::pair<double,double> >
activeCurveKnn(int init, int final, int step)
{
    return activeCurve<KnnModel>(init, final, step);
}

std::vector< std::pair<double,double> >
activeCurveSVM(int init, int final, int step)
{
    return activeCurve<SvmModel>(init, final, step);
}



template<class Model>
void convertQueriesToEdits(
    vector<EditDatum> &edits,
    const DataTable &batch,
    Model &model,
    const DataTable &gold_standard
) {
    edits.resize(batch.size());
    
    int i=0;
    for(DataTable::const_iterator it=batch.begin();
        it!= batch.end(); it++)
    {
        edits[i] = EditDatum(*it);
        edits[i].makeGuess(model);
        edits[i].simulateEdit(gold_standard);
        i++;
    }
}

void convertEditsToBasicData(
    DataTable &batch,
    const vector<EditDatum> &edits
) {
    vector<DataRow> rows(edits.size() * 2);
    
    int write = 0;
    for(uint i=0; i<edits.size(); i++) {
        rows[write].image     = edits[i].image0;
        rows[write].features  = edits[i].features0;
        rows[write].label     = edits[i].label0_solicited;
        rows[write].uid       = edits[i].uid0;
        write++;
        
        if(!edits[i].has_mod)   continue;
        
        rows[write].image     = edits[i].normed_mod_image;
        rows[write].features  = edits[i].mod_features;
        rows[write].label     = edits[i].mod_label;
        rows[write].uid       = UID::create();
        write++;
        if(edits[i].mod_features.size() == 0)
            cout << "OH FUCK FUCK " << endl;
    }
    rows.resize(write);
    
    batch.set(rows);
}

template<class Model> vector< pair<double,double> >
editCurve(int init, int final, int step)
{
    vector< pair<double, double> > results;
    srand(time(0));
    
    DataTable fullData;
    fullData.loadTrain();
    DataTable trainReserve(fullData);
    
    Model model(N_FEATURES, 10/*N_CLASSES*/);
    
    DataTable test;
    test.loadTest();
    
    
    // setup the inital dataset, and train a model on it
    DataTable train;
    trainReserve.removeRandomSubset(train, init);
    model.bulkTrain(&train);
    printLabelHistogram(train);
    
    
    // now we go into an evaluate->train loop
    while(int(train.size()) <= final) {
    // evaluate
        int correct = 0;
        for(DataTable::iterator it = test.begin(); it != test.end(); it++) {
            if(model.classify(it->features) == it->label)
                correct++;
        }
        results.push_back(pair<double,double>(train.size(), correct));
        cout << correct << endl;
    // abort?
        int oldsize = train.size();
        if(oldsize + step > final)  break;
    // sampling
        DataTable batch;
        trainReserve.removeRandomSubset(batch, step/2);
        //greedyBatchQuerySelection(batch, trainReserve, model, step/2);
        //dumbBatchQuerySelection(batch, trainReserve, model, step);
        
        // now, obtain edit information
        vector<EditDatum> edits;
        convertQueriesToEdits(edits, batch, model, fullData);
        convertEditsToBasicData(batch, edits);
    
    // re-training
        // now, add in the selected batch and re-train
        train.unionWith(batch);
        model.bulkTrain(&train);
        printLabelHistogram(train);
    }
    cout << "DONE!" << endl;
    
    return results;
}

vector< pair<double,double> >
editCurveLogistic(int init, int final, int step)
{
    return editCurve<LogisticModel>(init, final, step);
}

vector< pair<double,double> >
editCurveKnn(int init, int final, int step)
{
    return editCurve<KnnModel>(init, final, step);
}

vector< pair<double,double> >
editCurveSVM(int init, int final, int step)
{
    return editCurve<SvmModel>(init, final, step);
}


template<class Model>
int runOnPrefix(int samples)
{
    DataTable train;
    train.loadTrain(samples);
    
    Model model(N_FEATURES, 10/*N_CLASSES*/);
    model.bulkTrain(&train);
    
    // We've trained the model, now let's get some test data and
    // go to town with it.
    //printWeights(model);
    
    DataTable test;
    test.loadTest();
    
    int nTotal = test.size();
    int nRight = 0;
    int nWrong = 0;

    int counter = 0;
    for(DataTable::iterator it = test.begin(); it != test.end(); it++) {
        if(counter%1000 == 0) cout << "processed " << counter << endl;
        counter++;
        
        int prediction = model.classify(it->features);
        if(prediction == it->label)
            nRight++;
        else
            nWrong++;
    }
    
    cout << "total number of test images: " << nTotal << endl;
    cout << "        correct predictions: " << nRight << endl;
    cout << "                             ("
         << 100 * double(nRight)/nTotal << "%)" << endl;
    cout << "      incorrect predictions: " << nWrong << endl;
    cout << "                             ("
         << 100 * double(nWrong)/nTotal << "%)" << endl;
    
    
    return 0;
}

int runLogistic(int samples)
{
    return runOnPrefix<LogisticModel>(samples);
}

int runKNN(int samples)
{
    return runOnPrefix<KnnModel>(samples);
}

int runSVM(int samples)
{
    return runOnPrefix<SvmModel>(samples);
}



