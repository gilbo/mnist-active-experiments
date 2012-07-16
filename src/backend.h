
#pragma once

#include <cstdlib>
#include <vector>
#include <utility>
//using std::vector;
//using std::pair;

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

std::vector< std::pair<double,double> >
editCurveLogistic(int init, int final, int step);
std::vector< std::pair<double,double> >
editCurveKnn(int init, int final, int step);
std::vector< std::pair<double,double> >
editCurveSVM(int init, int final, int step);

int getImageSize(); // in count of bytes for buffer

void getTrainingImage(char *buffer, std::size_t size, int num);


