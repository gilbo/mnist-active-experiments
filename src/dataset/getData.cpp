
#include "getData.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace {
    using namespace std;
    
    string train_images_filename = "data/train-images-idx3-ubyte";
    string train_labels_filename = "data/train-labels-idx1-ubyte";
    
    string test_images_filename = "data/t10k-images-idx3-ubyte";
    string test_labels_filename = "data/t10k-labels-idx1-ubyte";
    
        
    inline
    uint flipEndian(uint val) {
        uint inval = val;
        uint ret;
        char *in    = (char *)(&inval);
        char *out   = (char *)(&ret);
        for(uint k=0; k<4; k++)
            out[4-k-1] = in[k];
        return ret;
    }
    
    struct ImageHeader {
        uint magic;
        uint nImages;
        uint nRows;
        uint nColumns;
        void flip() {
            magic   = flipEndian(magic);
            nImages = flipEndian(nImages);
            nRows   = flipEndian(nRows);
            nColumns = flipEndian(nColumns);
        }
    };
    
    // dump results into images
    // return false if problem while reading
    bool readImages(string filename, vector<Image> &images)
    {
        ifstream file(filename.c_str());
        if(!file)
            return false;
        
        // check the header and get number of items
        ImageHeader header;
        if(!file.read((char*)(&header),sizeof(ImageHeader)))
            return false;
        header.flip();
        int nImg = header.nImages;
        
        if(header.magic != 0x00000803 ||
           header.nRows != uint(Image::HEIGHT) ||
           header.nColumns != uint(Image::WIDTH)) {
            cerr << header.magic << endl;
            cerr << "bad header values" << endl;
            return false;
        }
        
        // now read out all of the images
        images.resize(nImg);
        for(int i=0; i<nImg; i++) {
            if(!file.read((char*)(&(images[i])), sizeof(Image)))
                return false;
        }
        
        return true;
    }
    
    
    
    struct LabelHeader {
        uint magic;
        uint nLabels;
        void flip() {
            magic   = flipEndian(magic);
            nLabels = flipEndian(nLabels);
        }
    };
    
    // dump results into labels
    // return false if problem while reading
    bool readLabels(string filename, vector<int> &labels)
    {
        ifstream file(filename.c_str());
        if(!file)
            return false;
        
        // check the header and get number of items
        LabelHeader header;
        if(!file.read((char*)(&header),sizeof(LabelHeader)))
            return false;
        header.flip();
        int nLabel = header.nLabels;
        
        if(header.magic != 0x00000801) {
            cerr << "bad header values" << endl;
            return false;
        }
        
        // now read out all of the images
        labels.resize(nLabel);
        char buf;
        for(int i=0; i<nLabel; i++) {
            if(!file.read(&buf, 1))
                return false;
            labels[i] = int(buf);
        }
        
        return true;
    }
    
} // end anonymous namespace



bool trainingData(std::vector<Image> &images, std::vector<int> &labels)
{
    if(!readImages(train_images_filename, images)) {
        cerr << "Had trouble reading " << train_images_filename << endl;
        return false;
    }
    if(!readLabels(train_labels_filename, labels)) {
        cerr << "Had trouble reading " << train_labels_filename << endl;
        return false;
    }
    if(images.size() != labels.size()) {
        cerr << "different numbers of labels and images" << endl;
        return false;
    }
    
    return true;
}

bool testingData(std::vector<Image> &images, std::vector<int> &labels)
{
    if(!readImages(test_images_filename, images)) {
        cerr << "Had trouble reading " << train_images_filename << endl;
        return false;
    }
    if(!readLabels(test_labels_filename, labels)) {
        cerr << "Had trouble reading " << train_labels_filename << endl;
        return false;
    }
    if(images.size() != labels.size()) {
        cerr << "different numbers of labels and images" << endl;
        cerr << "images: " << images.size() << endl;
        cerr << "labels: " << labels.size() << endl;
        return false;
    }
    
    return true;
}



