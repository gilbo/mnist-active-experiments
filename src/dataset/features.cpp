
#include "features.h"

// assumes the above amount of space has been allocated for features
void extractFeatures(vector<double> &features, const Image &image)
{
    features.resize(N_FEATURES);
    
    // just rescale the image to fit in the range 0 to 1 for now
    int f=0;
    for(int r=0; r<Image::HEIGHT; r++) {
        for(int c=0; c<Image::WIDTH; c++) {
            uchar val = image.p[r][c];
            features[f] = double(val) / 255.0;
            f += 1;
        }
    }
    features[f] = 1.0; // term to allow linear classifiers to encode bias
    
}