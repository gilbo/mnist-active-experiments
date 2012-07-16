
%module backend

%include <pybuffer.i>
%include "std_vector.i"
%include "std_pair.i"

%{
#include "backend.h"
%}

namespace std {
   %template(DoublePair) pair<double,double>;
   %template(DoublePairVector) vector< pair<double,double> >;
}

int runDevelop();

int runLogistic(int samples);
int runKNN(int samples);
int runSVM(int samples);

std::vector< std::pair<double,double> >
passiveCurveLogistic(int init, int final, int step);
std::vector< std::pair<double,double> >
passiveCurveKnn(int init, int final, int step);
std::vector< std::pair<double,double> >
passiveCurveSVM(int init, int final, int step);

std::vector< std::pair<double,double> >
activeCurveLogistic(int init, int final, int step);
std::vector< std::pair<double,double> >
activeCurveKnn(int init, int final, int step);
std::vector< std::pair<double,double> >
activeCurveSVM(int init, int final, int step);

int editCurveLogistic(int init, int final, int step);
int editCurveKnn(int init, int final, int step);
int editCurveSVM(int init, int final, int step);

int getImageSize(); // in count of bytes needed for buffer

%pybuffer_mutable_binary(char *buffer, size_t size1);
void getTrainingImage(char *buffer, size_t size1, int num);