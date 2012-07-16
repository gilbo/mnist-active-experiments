
#pragma once

#include "prelude.h"
#include "image.h"

#include <vector>

// retreive the specified data into the provided vector objects
bool trainingData(std::vector<Image> &images, std::vector<int> &labels);
bool testingData(std::vector<Image> &images, std::vector<int> &labels);